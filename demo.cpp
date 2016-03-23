#include <iostream>
#include <string.h>
#include "mrc.h"
using namespace std;

int main() {
    string base = "/home/linzichuan/Study/senior_second/particle/cryoEM-data/gammas-lowpass/";
    string file = base + "stack_0001_cor.mrc";
    string mode = "r";
    MRC m(file.c_str(), mode.c_str());
    int row = m.getNy();
    int col = m.getNx();
    int size = row * col;
    m.printInfo();
    cout << m.getImSize() << endl;
    float *buf = new float[size];
    m.read2DIm_32bit(buf, 0);
    /*FILE *fp;
    if ((fp = fopen("./test.bin", "wb")) == NULL) {
        fprintf(stderr, "Cannot output file\n");
        return 1;
    }
    fwrite(buf, sizeof(float), 3838*3710, fp);
    delete[] buf;
    fclose(fp);*/
    return 0;
}
