#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H

#include <stdint.h>

/* Multiboot2 specification constants */
#define MULTIBOOT2_MAGIC 0x36d76289
#define MULTIBOOT2_ARCHITECTURE_I386 0
#define MULTIBOOT2_HEADER_TAG_END 0
#define MULTIBOOT2_HEADER_TAG_INFORMATION_REQUEST 1

/* Multiboot2 header structure */
struct multiboot2_header {
    uint32_t magic;
    uint32_t architecture;
    uint32_t header_length;
    uint32_t checksum;
} __attribute__((packed));

/* Multiboot2 tag header */
struct multiboot2_tag {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
} __attribute__((packed));

/* Multiboot2 information structure passed to kernel */
struct multiboot2_info {
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed));

/* Memory map entry */
struct multiboot2_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} __attribute__((packed));

/* Memory map tag */
struct multiboot2_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot2_mmap_entry entries[0];
} __attribute__((packed));

#endif /* MULTIBOOT2_H */
