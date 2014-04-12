#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <opencv2\opencv.hpp>

/* Generate the file path using exist file index
 * input: indexfile name
 * output: a vector of string that contains the path of all the results */
void FileIndexConvert( std::string filelistname, std::vector<std::string> &filepath )
{
	std::ifstream resultlist(filelistname);
	std::vector<int> filelist;
	std::string fileindex;
	getline(resultlist, fileindex);
	std::istringstream sinput( fileindex );
	int index;
	while ( sinput >> index )
	{
		filelist.push_back(index);
	}
	resultlist.close();

	for ( int i = 0; i < filelist.size(); i++ )
	{
		std::string path_prefix = "LabelMe/test/";
		std::ostringstream convert;
		convert << path_prefix << "00";

		// folder level
		int folder_num = filelist[i] / 1000;
		if ( folder_num < 10 )
		{
			convert << "0";
		}
		convert << folder_num << "/" << "0";

		// filename level
		if ( folder_num < 10 )
		{
			convert << "0";
		}
		convert << folder_num;

		if ( filelist[i] % 1000 < 10 )
		{
			convert << "00";
		}
		else if ( filelist[i] % 1000 < 100 )
		{
			convert << "0";
		}
		convert << ( filelist[i] % 1000 ) << ".jpg";

		std::string temp = convert.str();

		filepath.push_back(temp);
	}
}

void LoadImg( std::vector<std::string> &filepath, std::vector<cv::Mat> &img )
{
	for ( int i = 0; i < filepath.size(); i++ )
	{
		cv::Mat temp = cv::imread( filepath[i], CV_LOAD_IMAGE_UNCHANGED );
		img.push_back(temp);
	}
}

/* Display the images in a grid
 * if the result number is less than or equals 4, all images will display in one row
 * the max col of the grid is 4
 * the max number of img that can be display is 12
 */
void DisplayImg( std::vector<cv::Mat> &img )
{
	int num_img = img.size();
	cv::Mat dispimg;
	std::string windowname = "Results";
	int size;

	// img col and row
	int x, y;

	// # of imgs display in a row and col
	int w, h;

	// resize scale
	float scale;
	int max;

	if ( num_img <= 0 )
	{
		std::cout << "No match records found!" << std::endl;
		return;
	}
	else if ( num_img <= 4 )
	{
		w = num_img,  h = 1;
		size = 300;
	}
	else if ( num_img > 4 && num_img <= 12 )
	{
		w = 4, h = (num_img + 3) / 4;
		size = 200;
	}
	// display top 12 images
	else if ( num_img > 12 )
	{
		w = 4, h = 3;
		size = 200;
		num_img = 12;
		windowname = "Top 12 Images";
	}

	dispimg.create( cv::Size( 60 + size*w, 20 + size*h ), CV_8UC3 );
	for ( int i = 0, m = 10, n = 10; i < num_img; i++, m += (10 + size) )
	{
		x = img[i].cols;
		y = img[i].rows;

		max = ( x > y )? x : y;
		scale = (float)( (float)max / size );
		if ( i % w == 0 && m != 10 )
		{
			m = 10;
			n += 10 + size;
		}

		cv::Mat imgROI = dispimg( cv::Rect( m, n, (int)(x/scale), (int)(y/scale) ) );
		cv::resize( img[i], imgROI, cv::Size((int)(x/scale), (int)(y/scale)) );
	}
	cv::imshow( windowname, dispimg );
}

int main( int argc, char* argv[] )
{
	std::vector<std::string> filepath;
	std::vector<cv::Mat> img;

	if ( argc != 2 )
	{
		std::cout << "Pass the result filename as the argument." << std::endl;
		return -1;
	}

	FileIndexConvert(argv[1], filepath);
	LoadImg( filepath, img );
	DisplayImg(img);
	//cv::waitKey(0);
	return 0;
}