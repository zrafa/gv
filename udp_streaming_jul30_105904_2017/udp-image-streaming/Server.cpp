/*
 *   C++ UDP socket server for live image upstreaming
 *   Modified from http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoServer.cpp
 *   Copyright (C) 2015
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "PracticalSocket.h" // For UDPSocket and SocketException
#include <iostream>          // For cout and cerr
#include <cstdlib>           // For atoi()
#include <iomanip>           // For setfill()

#define BUF_LEN 65507 

#include "opencv2/opencv.hpp"
using namespace cv;
#include "config.h"
static bool debug = false;

unsigned char buffer[BUF_LEN]; // Buffer for echo string

int main(int argc, char * argv[]) {
    int scale = 1;

    if (argc != 3) { // Test for correct number of parameters
        cerr << "Usage: " << argv[0] << " <Server Port>" << " <Scale> " << endl;
        exit(1);
    }

    scale = atoi(argv[2]);

    unsigned short servPort = atoi(argv[1]); // First arg:  local port

    namedWindow("recv", CV_WINDOW_AUTOSIZE);
    try {
        UDPSocket sock(servPort);

        int recvMsgSize; // Size of received message
        string sourceAddress; // Address of datagram source
        unsigned short sourcePort; // Port of datagram source
        struct header * header_pt = (struct header *)buffer;

 
        while (1) {
            // Block until receive message from a client
            do {
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, sourcePort);
                if (debug)
                    cout << "recvMsgSize " << recvMsgSize << endl;
            } while (recvMsgSize > sizeof(int));
            if (debug) {
                for (int i = 0; i < recvMsgSize; ++i)
                    cout << hex << setfill('0') << setw(2) << setprecision(2) << (int)buffer[i] << " ";
                    cout << endl;
            }
            int total_pack = ntohl(((int * ) buffer)[0]);
            if (debug) {
                cout << "total_pack: " << total_pack << endl;
            }
            char * longbuf = new char[PACK_SIZE * total_pack];
            for (int i = 0; i < total_pack; i++) {
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, sourcePort);
                if (recvMsgSize != PACK_SIZE) {
                    continue;
                }
                memcpy( & longbuf[i * PACK_SIZE], buffer, PACK_SIZE);
            }
 
            Mat rawData = Mat(1, PACK_SIZE * total_pack, CV_8UC1, longbuf);
            Mat frame = imdecode(rawData, CV_LOAD_IMAGE_COLOR);
            if (frame.size().width == 0) {
                if (debug) {
                    cout << "decode failure!" << endl;
                    continue;
                }
            }
            cv::resize(frame, frame, cv::Size(0, 0), scale, scale);
            imshow("recv", frame);
            free(longbuf);

            waitKey(1);
        }
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }

    return 0;
}
