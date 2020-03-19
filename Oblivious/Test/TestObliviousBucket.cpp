#include <unordered_map>
#include <vector>

#include "../Enclave/Sketch/ObliviousBucket.h"

using namespace std;

#define START_FILE_NO 1
#define END_FILE_NO 2

typedef vector<FIVE_TUPLE> TRACE;
TRACE traces[END_FILE_NO - START_FILE_NO + 1];

void ReadInTraces(const char *trace_prefix)
{
    for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt)
    {
        char datafileName[100];
        sprintf(datafileName, "%s%d.dat", trace_prefix, datafileCnt - 1);
        FILE *fin = fopen(datafileName, "rb");

        FIVE_TUPLE tmp_five_tuple{};
        traces[datafileCnt - 1].clear();
        while(fread(&tmp_five_tuple, 1, 13, fin) == 13)
        {
            traces[datafileCnt - 1].push_back(tmp_five_tuple);
        }
        fclose(fin);

        printf("Successfully read in %s, %ld packets\n", datafileName, traces[datafileCnt - 1].size());
    }
    printf("\n");
}


int main() {
    ReadInTraces("../data/");

#define TEST_D 2400
    ObliviousBucket<TEST_D> *ob;

    for(int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt) {
        unordered_map<string, int> Real_Freq;

        ob = new ObliviousBucket<TEST_D>();

        int packet_cnt = (int) traces[datafileCnt - 1].size();
        for(int i = 0; i < packet_cnt; ++i) {
            uint8_t swap_key[FLOW_KEY_SIZE];
            uint32_t count = 0;
            memset(swap_key, 0, FLOW_KEY_SIZE);
            ob->insert((uint8_t*)(traces[datafileCnt - 1][i].key), swap_key, count);

            string str((const char*)(traces[datafileCnt - 1][i].key), FLOW_KEY_SIZE);
            Real_Freq[str]++;
        }

        double ARE = 0;
        for(auto & it : Real_Freq) {
            uint8_t key[FLOW_KEY_SIZE];
            memcpy(key, (it.first).c_str(), FLOW_KEY_SIZE);
            int est_val = get_val(ob->query(key));

            int dist = std::abs(it.second - est_val);
            ARE += dist * 1.0 / (it.second);
        }

        printf("%d.dat: ARE=%.3lf\n", datafileCnt - 1, ARE / Real_Freq.size());

        delete ob;
        Real_Freq.clear();
    }
}