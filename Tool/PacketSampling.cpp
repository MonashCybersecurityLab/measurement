#include <string>
#include <unordered_map>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap/pcap.h>

#include "CommonUtil.h"

using namespace std;

int main(int argc, char **argv) {
    // load argument here
    if(argc - 1 != 3) {
        printf("Unmatched argument list\n");
        printf("Usage: PacketSampling <file_name> <sampling_period> <output_path>");
    } else {
        char err_buf[PCAP_ERRBUF_SIZE];
        pcap_t *handler = pcap_open_offline(argv[1], err_buf);

        if(handler == nullptr) {
            fprintf(stderr, "Error: %s\n", err_buf);
            return -1;
        }
        int sampling_period = stoi(argv[2]);
        int segmentation = 60 / sampling_period;
        unordered_map<string, int> flow_map[segmentation];

        pcap_pkthdr header{};
        const unsigned char *packet;

        int count = 0;
        packet = pcap_next(handler, &header);
        if(packet != nullptr) {
            // save the start time
            long start_time = header.ts.tv_sec;

            do {
                // read the timestamp to decide the output file
                timeval timestamp = header.ts;
                // flow id buffer
                char flow_id[13];
                // read ip and protocol info
                ip *IP = (ip*) packet;
                memcpy(flow_id, &IP->ip_src.s_addr, sizeof(uint32_t));
                memcpy(flow_id + sizeof(uint32_t), &IP->ip_dst.s_addr, sizeof(uint32_t));
                flow_id[12] = IP->ip_p;
                // read port info if it is TCP or UDP
                int IP_hdr_len = IP->ip_hl * 4;
                packet += IP_hdr_len;
                switch (IP->ip_p) {
                    case IPPROTO_TCP:
                    {
                        auto *TCP = (tcphdr*) packet;
                        memcpy(flow_id + 2 * sizeof(uint32_t), &TCP->th_sport, sizeof(uint16_t));
                        memcpy(flow_id + 2 * sizeof(uint32_t) + sizeof(uint16_t), &TCP->th_dport, sizeof(uint16_t));
                    }
                        break;
                    case IPPROTO_UDP:
                    {
                        auto *UDP = (udphdr*) packet;
                        memcpy(flow_id + 2 * sizeof(uint32_t), &UDP->uh_sport, sizeof(uint16_t));
                        memcpy(flow_id + 2 * sizeof(uint32_t) + sizeof(uint16_t), &UDP->uh_dport, sizeof(uint16_t));
                    }
                        break;
                    default:
                        break;
                }
                // get the id and store it
                string str_id(flow_id, 13);
                flow_map[(timestamp.tv_sec - start_time) / sampling_period][str_id]++;
                // counter
                count++;
            } while ((packet = pcap_next(handler, &header)) != nullptr);
        }
        printf("Num of packets: %d", count);

        FILE *map_fp;
        for(int i = 0; i < segmentation; i++) {
            // write the compressed list
            map_fp = fopen((string(argv[3]) +
                    to_string(sampling_period) + "/" + to_string(i + 1)).c_str(), "wb");
            for(auto & it : flow_map[i]) {
                uint8_t key[13];
                memcpy(key, (it.first).c_str(), 13);
                fwrite(key, 1, 13, map_fp);
                fwrite(&it.second, sizeof(int), 1, map_fp);
            }
            fclose(map_fp);
        }
    }
    return 0;
}