// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

// The buffer cache is a hash table of NBUCKET buckets, each with
// its own lock and LRU list of buffers, so that concurrent lookups
// of blocks that hash to different buckets don't contend for a
// single lock.  At most one copy of each block is cached: lookup
// and insertion for a block both happen under the lock of the
// bucket that the block hashes to.
#define NBUCKET 13

struct {
  struct spinlock eviction_lock;     // serializes eviction in bget
  struct spinlock bucket_lock[NBUCKET];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head[i].next is most recent, head[i].prev is least.
  struct buf head[NBUCKET];
} bcache;

int
bhash(uint blockno)
{
  return blockno % NBUCKET;
}

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.eviction_lock, "bcache");

  for(int i = 0; i < NBUCKET; i++) {
    initlock(&bcache.bucket_lock[i], "bcache.bucket");
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
  }

  // Create linked list of buffers, distributing them
  // among the buckets.
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    int bucket = (b - bcache.buf) % NBUCKET;
    b->next = bcache.head[bucket].next;
    b->prev = &bcache.head[bucket];
    initsleeplock(&b->lock, "buffer");
    bcache.head[bucket].next->prev = b;
    bcache.head[bucket].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int bucket = bhash(blockno);

  acquire(&bcache.bucket_lock[bucket]);

  // Is the block already cached?
  for(b = bcache.head[bucket].next; b != &bcache.head[bucket]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.bucket_lock[bucket]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bcache.bucket_lock[bucket]);

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  // Serialize eviction: it may need to move a buffer from one
  // bucket to another, taking two bucket locks.
  acquire(&bcache.eviction_lock);
  acquire(&bcache.bucket_lock[bucket]);

  // Check again, maybe another process cached the block.
  for(b = bcache.head[bucket].next; b != &bcache.head[bucket]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.bucket_lock[bucket]);
      release(&bcache.eviction_lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Recycle the least recently used (LRU) unused buffer
  // of this bucket.
  for(b = bcache.head[bucket].prev; b != &bcache.head[bucket]; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.bucket_lock[bucket]);
      release(&bcache.eviction_lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // All buffers of this bucket are in use.  Steal an unused
  // buffer from another bucket and move it to this bucket.
  for(int obucket = 0; obucket < NBUCKET; obucket++){
    if(obucket == bucket)
      continue;
    acquire(&bcache.bucket_lock[obucket]);
    for(b = bcache.head[obucket].prev; b != &bcache.head[obucket]; b = b->prev){
      if(b->refcnt == 0) {
        // Remove from the old bucket's list.
        b->next->prev = b->prev;
        b->prev->next = b->next;
        release(&bcache.bucket_lock[obucket]);
        // Insert at the head of the new bucket's list.
        b->next = bcache.head[bucket].next;
        b->prev = &bcache.head[bucket];
        bcache.head[bucket].next->prev = b;
        bcache.head[bucket].next = b;
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        release(&bcache.bucket_lock[bucket]);
        release(&bcache.eviction_lock);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcache.bucket_lock[obucket]);
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int bucket = bhash(b->blockno);
  acquire(&bcache.bucket_lock[bucket]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head[bucket].next;
    b->prev = &bcache.head[bucket];
    bcache.head[bucket].next->prev = b;
    bcache.head[bucket].next = b;
  }

  release(&bcache.bucket_lock[bucket]);
}

void
bpin(struct buf *b) {
  int bucket = bhash(b->blockno);
  acquire(&bcache.bucket_lock[bucket]);
  b->refcnt++;
  release(&bcache.bucket_lock[bucket]);
}

void
bunpin(struct buf *b) {
  int bucket = bhash(b->blockno);
  acquire(&bcache.bucket_lock[bucket]);
  b->refcnt--;
  release(&bcache.bucket_lock[bucket]);
}

