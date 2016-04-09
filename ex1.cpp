#include "mrc.h"
#include <QApplication>
#include <QPushButton>
#include <QImage>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <unordered_map>
#include <queue>
#include <vector>
#include <math.h>
#include <sstream>
#include <fstream>
#include <time.h>
#include <random>
#include <chrono>
#include <assert.h>
using namespace std;

//std::unordered_map<int,int> m;

void grayto256(float* gray, int *bmp, float max_, float min_, int size) {
    float delta = max_ - min_;
    float ratio = 255.0f / delta;
    for (int i = 0; i < size; ++i) {
        if (gray[i] < min_) {
            bmp[i] = 0;
        } else if (gray[i] > max_) {
            bmp[i] = 255;
        } else {
            bmp[i] = int((gray[i] - min_) * ratio);
        }
    }
    cout << "finish grayto256" << endl;
}

void graytoint(float* gray, int* gray_int, int size) {
    for (int i = 0; i < size; ++i) {
        gray_int[i] = int(gray[i]);
    }
}

/*bool read(float* buf, int size) {
    FILE *fp;
    char file[] = "/home/linzichuan/Study/senior_second/particle/test.bin";
    if ((fp = fopen(file, "r")) == NULL) {
        fprintf(stderr, "Cannot open test.bin");
        return false;
    }
    fread(buf, sizeof(float), size, fp);
    fclose(fp);
    return true;
}

void MyWidget::paintEvent(QPaintEvent *) {
    QPainter p;
    p.begin(this);
    p.drawLine(QLineF(1,1, 30, 30));
    p.end();
}*/

void re_arrange(int* in, int* out, int size, int z) {
    int l = sqrt(z);
    for (int i = 0; i < size; ++i) {
        int x = i / (88*l);
        int y = i % (88*l);
        int bx = x / 88;
        int by = y / 88;
        int ibx = x % 88;
        int iby = y % 88;
        int idx = (bx*l + by)*88*88 + (ibx*88 + iby);
        out[i] = in[idx];
    }
}

struct param{
    float mean;
    float standard;
};

struct param Gaussian_Distribution(float *whole, int size) {
    cout << size << endl;
    float mean = 0;
    for (int i = 0; i < size; ++i) {
        mean += whole[i];
    }
    mean = mean / size;
    cout << "mean = " << mean << endl;
    float variance = 0;
    for (int i = 0; i < size; ++i) {
        variance += (whole[i] - mean) * (whole[i] - mean);
    }
    float standard = sqrt(variance / size);
    cout << "standard = " << standard << endl;
    struct param p = {mean, standard};
    return p;
}
struct star_point {
    int x;
    int y;
};
struct star_ar {
    star_point* p;
    int length;
};

struct star_ar readstar(string starfile) {
    int num = 0;
    star_point *p = new star_point[300];
    ifstream fin(starfile);
    string s;
    for (int i = 0; i < 12; ++i) {
        fin >> s;
        //cout << s << endl;
    }
    string a, b;
    int ia, ib;
    string s1,s2,s3;
    while(fin >> a >> b >> s1 >> s2 >> s3) {
        stringstream ss;
        ss << a;
        ss >> ia;
        stringstream ss1;
        ss1 << b;
        ss1 >> ib;
        p[num++] = {ia, ib};
        //cout << ia << " " << ib << endl;
    }
    fin.close();
    struct star_ar sa = {p, num};
    return sa;
}

struct star_ar createnoisestar(int row, int col, int side, star_ar &star_points) {
    int size = star_points.length;
    int num = 0;
    assert(size < 400);
    int noise_point_size = size;  // to scale, 4 times the size
    star_point *p = new star_point[noise_point_size];
    vector<int> vr;
    vector<int> vc;
    int half = side*2;
    for (int i = half; i < row-half; ++i)
        vr.push_back(i);
    for (int i = half; i < col-half; ++i)
        vc.push_back(i);

    unsigned seed = std::chrono::system_clock::now ().time_since_epoch ().count ();
    std::shuffle (vr.begin (), vr.end (), std::default_random_engine (seed));
    seed = std::chrono::system_clock::now ().time_since_epoch ().count ();
    std::shuffle (vc.begin (), vc.end (), std::default_random_engine (seed));
    int i = 0;

    while (num < noise_point_size && i < vr.size()) {
        bool overlap = false;
        for (int j = 0; j < size; ++j) {
            int deltax = vr[i] - star_points.p[j].x;
            int deltay = vc[i] - star_points.p[j].y;
            if (deltax * deltax + deltay * deltay < 50 * 50) {
                overlap = true;
                break;
            }
        }
        if (!overlap) {
            p[num++] = {vr[i], vc[i]};
        }
        i++;
    }
    vr.clear();
    vc.clear();
    struct star_ar sa = {p, num};
    return sa;
}

void paint(int &row, int &col, int &z, int *bmp, star_ar &star_array, star_ar &noise_array, int &side) {
    cout << "start paiting..." << endl;
    QGraphicsScene* scene = new QGraphicsScene();

    int w_row = row*sqrt(z);
    int w_col = col*sqrt(z);
    scene->setSceneRect(0, 0, w_col, w_row);
    QImage img(w_row, w_col, QImage::Format_RGB888);
    cout << img.width() << endl << img.height() << endl;
    QPainter painter(&img);
    cout << w_row << " " << w_col << endl;
    for (int i = 0; i < w_row; ++i) {
        for (int j = 0; j < w_col; ++j) {
            int idx = i*w_col + j;
            painter.fillRect(QRect(i,j, 1,1), QColor(bmp[idx], bmp[idx], bmp[idx]));
        }
    }
    painter.setPen(QPen(Qt::green, 4, Qt::SolidLine));
    for (int i = 0; i < star_array.length; ++i) {
        painter.drawRect(star_array.p[i].y-side/2, star_array.p[i].x-side/2, side, side);
    }
/*
    painter.setPen(QPen(Qt::red, 4, Qt::SolidLine));
    ifstream fin("scan_indices");
    int index, num=0;
    int *indices = new int[1000];
    while (fin >> index) {
        cout << index << endl;
        indices[num++] = index;
    }
    fin.close();
    for (int i = 0; i < num; ++i) {
        int index = indices[i];
        int x = index / (col/100) * 100;
        int y = index % (col/100) * 100;
        painter.drawRect(x, y, side, side);
    }*/
    //cout << "num = " << num << endl;
    /*for (int i = 0; i < noise_array.length; ++i) {
        painter.drawRect(noise_array.p[i].y-side/2, noise_array.p[i].x-side/2, side, side);
    }*/
    scene->render(&painter);
    //QGraphicsView view(scene);
    //view.show();
    bool b = img.save("positive.png");
    cout << "finish paiting..." << endl;

}

void re() {

    /*FILE *fp;
    if ((fp = fopen("./36_sample.bin", "wb")) == NULL) {
        cout << "open sample file ERROR!" << endl;
    }
    fwrite(bmp, sizeof(float), size, fp);
    cout << "Write into sample file..." << endl;
    fclose(fp);
    //re arrange bmp array to 25 images
    int *bmp_align = new int[size];
    //re_arrange(bmp, bmp_align, size, z);
    */
}

void store(star_ar &star_array, int side, star_ar &noise_array, int *bmp, int fi, int col) {
    //<<<<<<<<<<<<<store positive samples>>>>>>>>>>>
    int *star = new int[star_array.length*side*side];
    int num = 0;
    cout << "start convert star_array" << endl;
    for (int i = 0; i < star_array.length; ++i) {
        int x0 = star_array.p[i].y - side/2;
        int y0 = star_array.p[i].x - side/2;
        //uncomment this with 4 times star array length
        /*for (int j = 0; j < side; ++j) {
            for (int k = 0; k < side; ++k) {
                int x = x0 + k;
                int y = y0 + 99-j;
                star[num++] = bmp[x*col + y];
            }
        }
        for (int j = 0; j < side; ++j) {
            for (int k = 0; k < side; ++k) {
                int x = x0 + 99-k;
                int y = y0 + j;
                star[num++] = bmp[x*col + y];
            }
        }
        for (int j = 0; j < side; ++j) {
            for (int k = 0; k < side; ++k) {
                int x = x0 + 99-j;
                int y = y0 + 99-k;
                star[num++] = bmp[x*col + y];
            }
        }*/
        for (int j = 0; j < side; ++j) {
            for (int k = 0; k < side; ++k) {
                int x = x0 + j;
                int y = y0 + k;
                star[num++] = bmp[x*col + y];
            }
        }
    }
    cout << "finish get positive patch" << endl;
    stringstream ss;
    string sfi;
    ss << fi;
    ss >> sfi;
    //string wfile = "./spliceosome_bin/spliceosome_star_"+sfi+".bin";
    string wfile = "./gammas_gray/star_"+sfi+".bin";
    FILE *fp1;
    if ((fp1 = fopen(wfile.c_str(), "wb")) == NULL) {
        cout << "open write positive file ERROR!" << endl;
    }
    cout << star_array.length * side * side << endl;
    fwrite(star, sizeof(int), num, fp1);
    fclose(fp1);
    cout << "write positive success" << endl;

    //<<<<<<<<<<<<store negative samples>>>>>>>>>>>>>
    int *noise = new int[noise_array.length*side*side];
    int unum = 0;
    for (int i = 0; i < noise_array.length; ++i) {
        int x0 = noise_array.p[i].y - side/2;
        int y0 = noise_array.p[i].x - side/2;
        for (int j = 0; j < side; ++j) {
            for (int k = 0; k < side; ++k) {
                int x = x0 + j;
                int y = y0 + k;
                noise[unum++] = bmp[x*col + y];
            }
        }
    }
    cout << "finish get negative patch" << endl;
    stringstream ss2;
    string sfi1;
    ss2 << fi;
    ss2 >> sfi1;
    string wfile1 = "./gammas_gray/noise_"+sfi1+".bin";
    FILE *fp2;
    if ((fp2 = fopen(wfile1.c_str(), "wb")) == NULL) {
        cout << "open write noise file ERROR!" << endl;
    }
    cout << star_array.length * side * side << endl;
    fwrite(noise, sizeof(int), unum, fp2);
    fclose(fp2);
    cout << "write negative success" << endl;

    delete star;
    delete noise;
}

int main (int argc, char *argv[]) {

    string base = "/home/linzichuan/Study/senior_second/particle/cryoEM-data/gammas-lowpass/";
    //string file = base + "gammas-lowpass/stack_0001_cor.mrc"; //"test_2x_10000.mrc";
    //string starfile = base + "gammas-lowpass/stack_0001_cor_manual_lgc.star";
    string manual_files = "/home/linzichuan/Study/senior_second/particle/manual_files.txt";
    string images_files = "/home/linzichuan/Study/senior_second/particle/images_with_star.txt";
    string* origfiles = new string[500];
    string* starfiles = new string[500];
    int files_num = 0;
    string ns, ns1;
    ifstream fin1(manual_files);
    ifstream fin2(images_files);
    while(fin1 >> ns && fin2 >> ns1) {
        starfiles[files_num] = ns;
        origfiles[files_num] = ns1;
        files_num ++;
    }
    cout << "files_num = " << files_num << endl;
    fin1.close();
    fin2.close();

    int end = 1;
    //int fi = 0;
    for (int fi = 0; fi < end; ++fi) {
        cout << "starting " << fi << endl;
        string starfile = base + starfiles[fi];
        string file = base + origfiles[fi];
        cout << "==>> " << file << endl;
        cout << "==>>" << starfile << endl;

        int dis_size = 1;
        int z = 1; //36
        string mode = "r";
        MRC m(file.c_str(), mode.c_str());
        int row = m.getNy();
        int col = m.getNx();
        int size = row * col * z;
        //m.printInfo();
        cout << m.getImSize() << endl;
        float *gray = new float[size];
        for (int i = 0; i < z; ++i) {
            m.read2DIm_32bit(gray + i*row*col, i);
        }

        float *whole = new float[dis_size*row*col];
        for (int i = 0; i < dis_size; ++i) {
            m.read2DIm_32bit(whole + i*row*col, i);
        }
        //Gaussian Distribution
        struct param p = Gaussian_Distribution(whole, dis_size*row*col);
        delete[] whole;
        float min_ = p.mean - 3*p.standard;
        float max_ = p.mean + 3*p.standard;
        cout << "min = " << min_ << ", max = " << max_ << endl;

        int *bmp = new int[size];
        grayto256(gray, bmp, max_, min_, size);
        int *gray_int = new int[size];
        graytoint(gray, gray_int, size);

        int side = 100;
        cout << "row = " << row << ", col = " << col << endl;
        star_ar star_array = readstar(starfile);
        star_ar noise_array = createnoisestar(row, col, side, star_array);
        cout << "stars number = " << star_array.length << endl;
        cout << "noise number = " << noise_array.length << endl;

        cout << "next loop..." << endl;

        //QApplication app(argc, argv);
        //paint(row, col, z, bmp, star_array, noise_array, side);
        //store(star_array, side, noise_array, bmp, fi, col);


        delete gray;
        delete bmp;
        //return app.exec();
    }

    cout << "out loop..." << endl;
    //Binning(mean pooling)
    //TODO

    //return 0;
}
