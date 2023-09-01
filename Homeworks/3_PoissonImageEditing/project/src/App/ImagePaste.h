#pragma once
#include "ScanLine.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <eigen3/Eigen/Sparse>
#include<eigen3/Eigen/IterativeLinearSolvers>

class ImagePaste
{
public:
	ImagePaste();
	~ImagePaste();
	void poisson_init(cv::Mat source_img);
	void set_inside_mask(Eigen::MatrixXi inside_mask);
	void paste_poisson(QPoint paste_point, QPoint source_point, cv::Mat& paste_img_, cv::Mat& source_img_);
	void paste_mixing_poisson(QPoint paste_point, QPoint source_point, cv::Mat& paste_img, cv::Mat& source_img);
	void paste_normal(QPoint paste_point, QPoint source_point, cv::Mat& paste_img, cv::Mat& source_img);

private:
	double norm(cv::Vec3i vec);

private:

	int pixels_cnt;
	Eigen::MatrixXi figure_inside_index_mat;
	int mask_width, mask_height;
	Eigen::SparseMatrix<float> diffrent_sparse_matrix;
	Eigen::SimplicialLLT<Eigen::SparseMatrix<float>> solver;
	Eigen::MatrixXi figure_inside_mask;
	Eigen::VectorXf div_R;
	Eigen::VectorXf div_G;
	Eigen::VectorXf div_B;
};