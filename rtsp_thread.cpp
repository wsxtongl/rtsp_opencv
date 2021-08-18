#include <iostream>
#include <stdio.h>
#include <boost/thread.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <time.h>
#include <queue>
using namespace std;
using namespace cv;

boost::mutex mtx;
queue < cv::Mat > msgq;


void camCapture(string& cap_url, Mat& frame, bool* Capture)
{
	VideoCapture cap(cap_url);
	if (cap.isOpened())  // check if we succeeded
		;

	long send_qsize;
	while (*Capture == true)
	{
		mtx.lock();
		cap >> frame;
		
		if (frame.empty())
		{
			printf("Can't receive frame. Retrying ...");
			cap.release();
			VideoCapture cap2(cap_url);
			if (cap2.isOpened())  // check if we succeeded
				printf("open");
			cap2 >> frame;
			cap = cap2;
			cap2.release();
			//cout << int(frame.rows) << endl;
		}
		else
		{
			msgq.push(frame);
		}
		
		mtx.unlock();

		
		

		//cout << "*****" << endl;
		send_qsize = msgq.size();
		if (send_qsize > 3)
		{
			msgq.pop();

		}
		Sleep(10);
		/*if (frame.empty())
		{
			; //cout << "No hay captura" << endl;
		}
		else
		{
			;//cout << "Frame exist" << endl;
		}*/

	}
	cout << "camCapture finished\n";

	return;
}

int main() {

	try {
		string cap_url = "rtsp://admin:Otn2020!@172.27.21.65:554/h264/ch1/main/av_stream";
		//VideoCapture cap(cap_url); // open the default camera
		Mat frame, current_frame, SFI, Input,dst;
		bool *Capture = new bool; // better not use a pointer here, but use a bool and pass the address or by reference.
		*Capture = true;
		//if (!cap.isOpened())  // check if we succeeded
		//	return -1;
		//your capture thread has started
		boost::thread captureThread(camCapture, boost::ref(cap_url), boost::ref(frame), Capture);
		long qsize;
		while (1)
		{
			if (msgq.empty())
			{
				Sleep(10); // sleep 0.01 sec before trying again
				continue;
			}
			else

				mtx.lock();
				//current_frame = frame.clone();
				qsize = msgq.size();
				cout << "queue_num: "<< qsize << endl;
				//if (qsize > 5)
					//cout << "Queue size: " << qsize << endl;
				current_frame = msgq.front(); // get next message in queue
				msgq.pop();
				mtx.unlock();

			






			if (current_frame.empty())
			{
				cout << "Current_Frame empty" << endl;
			}

			else 
			{

				
				cv::resize(frame, dst, cv::Size(int(current_frame.cols / 2), int(current_frame.rows / 2)));
				

				imshow("Streaming", dst);
				waitKey(1);
				Sleep(2000);
				/*if (waitKey(10) == 27)
				{
					// TODO: you should use a mutex (or an atomic type) here, too, maybe setting a bool is thread-safe, but this can't be guaranteed for each hardware!
					*Capture = false;
					break;
				}*/
			}
		}
		//Terminate the thread
		captureThread.join();

		// give memory free:
		delete Capture;
	}
	catch (Exception & e)
	{
		cout << e.what() << endl;
	}

	return 0;
}