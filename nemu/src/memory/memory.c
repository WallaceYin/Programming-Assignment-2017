#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define PDX(va)     (((uint32_t)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va)     (((uint32_t)(va) >> PTXSHFT) & 0x3ff)
#define OFF(va)     ((uint32_t)(va) & 0xfff)
#define PTXSHFT   12      // Offset of PTX in a linear address
#define PDXSHFT   22      // Offset of PDX in a linear address

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  uint32_t data_read;
  if (is_mmio(addr) == -1)
    data_read = pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  else{
    data_read = mmio_read(addr, len,is_mmio(addr));
  }

  return data_read;
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  if (is_mmio(addr) == -1)
    memcpy(guest_to_host(addr), &data, len);
  else
  {
    mmio_write(addr, len, data, is_mmio(addr));
  }

}
paddr_t page_translate(vaddr_t);

uint32_t vaddr_read(vaddr_t addr, int len) {
  if (cpu.cr0.PG == 0)
    return paddr_read(addr, len);
  if (((uint32_t)(addr & 0xfff) + (uint32_t)len) > 0x1000)
  {
    uint32_t paddr_1 = page_translate(addr);
    uint32_t paddr_2 = page_translate((uint32_t)addr + (0x1000 - (uint32_t)(addr & 0xfff)));
    uint32_t len_1 = 0x1000 - (uint32_t)(addr & 0xfff);
    uint32_t len_2 = len - len_1;
    return paddr_read(paddr_1, len_1) | paddr_read(paddr_2, len_2) << (8 * len_1);
  }
  else
  {
    uint32_t paddr = page_translate(addr);
    return paddr_read(paddr, len);
  }
  assert(0);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if (cpu.cr0.PG == 0)
  {
    paddr_write(addr, len, data);
    return;
  }
  if (((uint32_t)(addr & 0xfff) + (uint32_t)len) > 0x1000)
  {
    uint32_t paddr_1 = page_translate(addr);
    uint32_t paddr_2 = page_translate((uint32_t)addr + (0x1000 - (uint32_t)(addr & 0xfff)));
    uint32_t len_1 = 0x1000 - (uint32_t)(addr & 0xfff);
    uint32_t len_2 = len - len_1;
    paddr_write(paddr_1, len_1, (data << (32 - 8 * len_1)) >> (32 - 8 * len_1));
    paddr_write(paddr_2, len_2, (data >> (8 * len_1)));
    assert(0);
  }
  else
  {
    uint32_t paddr = page_translate(addr);
    paddr_write(paddr, len, data);
  }
}

paddr_t page_translate(vaddr_t addr) {
  uint32_t Dir_entry;
  Dir_entry = paddr_read(((PDX(addr) << 2) | ((cpu.cr3.addr & 0xfffff000) << 12)), 4);
  if ((Dir_entry & 0x001) == 0)
    Log("addr = %x",addr);
  assert((Dir_entry & 0x001) > 0);
  uint32_t Tab_entry;
  Tab_entry = paddr_read(((PTX(addr) << 2) | (Dir_entry & 0xfffff000)), 4);
  assert((Tab_entry & 0x001) > 0);
  return ((Tab_entry & 0xfffff000) | OFF(addr));
}
