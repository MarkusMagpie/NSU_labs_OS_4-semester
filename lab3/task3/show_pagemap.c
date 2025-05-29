#include <stdio.h>
#include <stdlib.h> // atoi, exit
#include <fcntl.h> // sysconf, read, close, lseek
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    int pid = atoi(argv[1]); // PID процесса
    char maps_path[256], pagemap_path[256];
    // записываю в maps_path путь к файлу maps, а в pagemap_path путь к pagemap
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);
    snprintf(pagemap_path, sizeof(pagemap_path), "/proc/%d/pagemap", pid);

    // открыл файл proc/pid/maps для чтения регионов памяти, получаю ссылку на него
    FILE *maps_file = fopen(maps_path, "r");
    // открыл файл proc/pid/pagemap, получаю его дескриптор
    int pagemap_fd = open(pagemap_path, O_RDONLY);

    if (!maps_file || pagemap_fd == -1) {
        printf("failed to fopen/open files: %s, %s\n:", maps_path, pagemap_path);
        return 1;
    }

    // sysconf(_SC_PAGESIZE) - размер страницы памяти в байтах 
    long page_size = sysconf(_SC_PAGESIZE);
    char line[256];

    // построчное чтение регионов из maps 
    while (fgets(line, sizeof(line), maps_file)) {
        unsigned long long start, end;
        // извлекаем из строки начальный (start) и конечный (end) виртуальные адреса региона памяти
        // ЦЕЛЬ: оппределить границы регионов памяти для последующей обработки
        if (sscanf(line, "%llx-%llx", &start, &end) != 2) {
            continue;
        }

        printf("REGION: %llx-%llx\n", start, end);

        // обработка каждой страницы в регионе памяти от start до end
        for (uint64_t vaddr = start; vaddr < end; vaddr += page_size) {
            // вычисляет смещение в файле pagemap для текущей страницы vaddr
            /*
            каждая страница va соответствует 64-битной записи в pagemap
                vaddr / page_size - номер страницы (для адреса 0x1000 (16) и page_size=4096 (10) это 1)
                * sizeof(uint64_t) - размер одной записи
            */
            uint64_t offset = (vaddr / page_size) * sizeof(uint64_t);

            // установил позицию чтения в файле pagemap на рассчитанный offset (SEEK_SET - от начала файла)
            if (lseek(pagemap_fd, offset, SEEK_SET) == -1) {
                printf("lseek failed");
                continue;
            }

            // read-ом читаем 8 байт из файла pagemap в entry
            uint64_t entry;
            if (read(pagemap_fd, &entry, sizeof(entry)) != sizeof(entry)) {
                printf("read failed");
                continue;
            }

            // present (63-й бит) - флаг присутствия страницы в RAM (1) или swap (0) (в файле подкачки, либо не загружена)
            // (entry >> 63) & 1 - сдвигаем биты на 63 позиции вправо и изолирую нулевой - получаю значение старшего
            int present = (entry >> 63) & 1;

            // pfn(physical frame number) - номер физической страницы в памяти
            /*
                1ULL << 55 - создал число 0x200000000000000 (1 в 55-й позиции)
                (1ULL << 55)-1 - -1 превращает это в маску из 55 единиц
                entry & - обнулили все биты, кроме младших 55
            */
            uint64_t pfn = entry & ((1ULL << 55) - 1);

            // PRIx64 — макрос для корректного вывода 64-битных чисел в шестнадцатеричном формате
            printf("\tVA: 0x%" PRIx64 " -> PFN: 0x%" PRIx64 " | present: %d\n", vaddr, pfn, present);
        }
    }

    fclose(maps_file);
    close(pagemap_fd);

    return 0;
}