#include <netinet/if_ether.h>
#include <pcap/pcap.h>

int main() {
    char err_buf[PCAP_ERRBUF_SIZE];
    pcap_t *handler = pcap_open_offline("equinix-nyc.dirA.20180315-130000.UTC.anon.pcap", err_buf);

    if(handler == nullptr) {
        fprintf(stderr, "Error: %s\n", err_buf);
        return -1;
    }

    pcap_pkthdr header{};
    const unsigned char *packet;

    int count = 0;
    while ((packet = pcap_next(handler, &header)) != nullptr) {
        // read the timestamp to decide the output file
        timeval timestamp = header.ts;

        // read IP address
        packet += sizeof(ether_header);
        count++;
    }

    printf("Num of packets: %d", count);
}