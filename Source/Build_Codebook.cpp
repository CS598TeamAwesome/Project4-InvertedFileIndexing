#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <Util\Distances.hpp>


std::vector<std::string> load_images(std::string imageFileList)
{
	std::ifstream iss(imageFileList);

	std::vector<std::string> images;
	while (iss)
	{
		std::string imFile;
		std::getline(iss, imFile);

		if (imFile.length() != 0)
			images.push_back(imFile);
	}

	iss.close();
	return images;
}

int main()
{
	std::vector<std::string> filename;

	filename = load_images("all_images");
	std::cout << "The size of Filelist is: " << filename.size() << std::endl;

	cv::SiftDescriptorExtractor extractor;
	cv::SiftFeatureDetector detector(200);
	std::vector<cv::Mat> training_descriptors;
	std::vector<std::vector<double>> samples;
	std::ofstream output("codebook_10k_128");
	for (int i = 0; i < filename.size(); i++)
	{
		cv::Mat img;
		std::cout << filename[i] << ": ";
		img = cv::imread(filename[i], CV_LOAD_IMAGE_UNCHANGED);
		std::vector<cv::KeyPoint> training_keypoints;
		detector.detect(img, training_keypoints);
		std::cout << training_keypoints.size() << std::endl;

		cv::Mat descriptor_uchar;
		extractor.compute(img, training_keypoints, descriptor_uchar);

		cv::Mat descriptor_double;
		descriptor_uchar.convertTo(descriptor_double, CV_64F);

		training_descriptors.push_back(descriptor_double);

		LocalDescriptorAndBagOfFeature::convert_mat_to_vector(descriptor_double, samples);

	}
	for (int i = 0; i < samples.size(); i++)
	{
		for ( int j = 0; j < samples[i].size(); j++ )
		{
			output << samples[i][j] << " ";
		}
		output << std::endl;
	}
	output.close();
	return 0;
}