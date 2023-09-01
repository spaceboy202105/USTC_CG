#include "ImagePaste.h"
#include <ctime>

ImagePaste::ImagePaste()
{
	pixels_cnt = 0;
	figure_inside_index_mat.resize(0, 0);
	diffrent_sparse_matrix.resize(0, 0);
}

ImagePaste::~ImagePaste()
{
}

void ImagePaste::set_inside_mask(Eigen::MatrixXi inside_mask)
{
	figure_inside_mask = inside_mask;
}

void ImagePaste::poisson_init(cv::Mat source_img)
{
	mask_width = figure_inside_mask.rows();
	mask_height = figure_inside_mask.cols();
	figure_inside_index_mat.resize(mask_width, mask_height);
	figure_inside_index_mat.setZero();
	for (int i = 0; i < mask_width; i++)
	{
		for (int j = 0; j < mask_height; j++)
		{
			if (figure_inside_mask(i, j) == 1)
			{
				figure_inside_index_mat(i, j) = pixels_cnt;
				pixels_cnt++;
			}
		}
	}

	diffrent_sparse_matrix.resize(pixels_cnt, pixels_cnt);
	diffrent_sparse_matrix.setZero();

	QVector<Eigen::Triplet<float>> coef;
	for (int i = 0; i < mask_width; i++)
	{
		for (int j = 0; j < mask_height; j++)
		{
			if (figure_inside_mask(i, j) == 1)
			{
				int index = figure_inside_index_mat(i, j);
				coef.push_back(Eigen::Triplet<float>(index, index, 4));
				if (i > 0 && figure_inside_mask(i - 1, j) == 1)
				{
					coef.push_back(Eigen::Triplet<float>(index, figure_inside_index_mat(i - 1, j), -1));
				}
				if (j > 0 && figure_inside_mask(i, j - 1) == 1)
				{
					coef.push_back(Eigen::Triplet<float>(index, figure_inside_index_mat(i, j - 1), -1));
				}
				if (i < mask_width - 1 && figure_inside_mask(i + 1, j) == 1)
				{
					coef.push_back(Eigen::Triplet<float>(index, figure_inside_index_mat(i + 1, j), -1));
				}
				if (j < mask_height - 1 && figure_inside_mask(i, j + 1) == 1)
				{
					coef.push_back(Eigen::Triplet<float>(index, figure_inside_index_mat(i, j + 1), -1));
				}
			}
		}
	}
	diffrent_sparse_matrix.setFromTriplets(coef.begin(), coef.end());
	diffrent_sparse_matrix.makeCompressed();

	solver.compute(diffrent_sparse_matrix);
	if (solver.info() != Eigen::Success)
	{
		throw std::exception("Compute Matrix is error");
		return;
	}
}

void ImagePaste::paste_poisson(QPoint paste_point, QPoint source_point, cv::Mat& paste_img_, cv::Mat& source_img_)
{
	div_R.resize(pixels_cnt);
	div_G.resize(pixels_cnt);
	div_B.resize(pixels_cnt);
	div_R.setZero();
	div_G.setZero();
	div_B.setZero();

	for (int i = 0; i < mask_width; i++)
	{
		for (int j = 0; j < mask_height; j++)
		{
			if (figure_inside_mask(i, j) == 1)
			{
				int index = figure_inside_index_mat(i, j);
				int x = source_point.y() + i;
				int y = source_point.x() + j;
				cv::Vec3i temp_vec = source_img_.at<cv::Vec3b>(x, y);
				temp_vec *= 4;
				temp_vec -= source_img_.at<cv::Vec3b>(x + 1, y);
				temp_vec -= source_img_.at<cv::Vec3b>(x - 1, y);
				temp_vec -= source_img_.at<cv::Vec3b>(x, y - 1);
				temp_vec -= source_img_.at<cv::Vec3b>(x, y + 1);

				div_R(figure_inside_index_mat(i, j)) += temp_vec[0];
				div_G(figure_inside_index_mat(i, j)) += temp_vec[1];
				div_B(figure_inside_index_mat(i, j)) += temp_vec[2];

				if (i == 0 || (i > 0 && !figure_inside_mask(i - 1, j)))
				{
					div_R[index] += paste_img_.at<cv::Vec3b>(i + paste_point.y() - 1, j + paste_point.x())[0];
					div_G[index] += paste_img_.at<cv::Vec3b>(i + paste_point.y() - 1, j + paste_point.x())[1];
					div_B[index] += paste_img_.at<cv::Vec3b>(i + paste_point.y() - 1, j + paste_point.x())[2];
				}
				if (i == mask_width - 1 || (i < mask_width - 1 && !figure_inside_mask(i + 1, j)))
				{
					div_R[index] += paste_img_.at<cv::Vec3b>(i + paste_point.y() + 1, j + paste_point.x())[0];
					div_G[index] += paste_img_.at<cv::Vec3b>(i + paste_point.y() + 1, j + paste_point.x())[1];
					div_B[index] += paste_img_.at<cv::Vec3b>(i + paste_point.y() + 1, j + paste_point.x())[2];
				}
				if (j == 0 || (j > 0 && !figure_inside_mask(i, j - 1)))
				{
					div_R[index] += paste_img_.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x() - 1)[0];
					div_G[index] += paste_img_.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x() - 1)[1];
					div_B[index] += paste_img_.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x() - 1)[2];
				}
				if (j == mask_height - 1 || (j < mask_height - 1 && !figure_inside_mask(i, j + 1)))
				{
					div_R[index] += paste_img_.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x() + 1)[0];
					div_G[index] += paste_img_.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x() + 1)[1];
					div_B[index] += paste_img_.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x() + 1)[2];
				}
			}
		}
	}

	Eigen::VectorXf vec_red(pixels_cnt), vec_green(pixels_cnt), vec_blue(pixels_cnt);

	vec_red = solver.solve(div_R);
	vec_green = solver.solve(div_G);
	vec_blue = solver.solve(div_B);

	for (int i = 0; i < mask_width; i++)
	{
		for (int j = 0; j < mask_height; j++)
		{
			if (figure_inside_mask(i, j) == 1)
			{
				int index = figure_inside_index_mat(i, j);
				int red = vec_red(index);
				int green = vec_green(index);
				int blue = vec_blue(index);
				paste_img_.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x())[0] = red > 255 ? 255 : (red < 0 ? 0 : red);
				paste_img_.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x())[1] = green > 255 ? 255 : (green < 0 ? 0 : green);
				paste_img_.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x())[2] = blue > 255 ? 255 : (blue < 0 ? 0 : blue);
			}
		}
	}
}

double ImagePaste::norm(cv::Vec3i vec)
{
	return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

void ImagePaste::paste_mixing_poisson(QPoint paste_point, QPoint source_point, cv::Mat& paste_img, cv::Mat& source_img)
{
	div_R.resize(pixels_cnt);
	div_G.resize(pixels_cnt);
	div_B.resize(pixels_cnt);
	div_R.setZero();
	div_G.setZero();
	div_B.setZero();

	for (int i = 0; i < mask_width; i++)
	{
		for (int j = 0; j < mask_height; j++)
		{
			if (figure_inside_mask(i, j))
			{
				int index = figure_inside_index_mat(i, j);
				int x1 = source_point.y() + i;
				int y1 = source_point.x() + j;
				int x2 = paste_point.y() + i;
				int y2 = paste_point.x() + j;
				cv::Vec3i vec, temp_vec, temp_vec_paste;

				temp_vec = source_img.at<cv::Vec3b>(x1, y1);
				temp_vec -= source_img.at<cv::Vec3b>(x1 + 1, y1);
				temp_vec_paste = paste_img.at<cv::Vec3b>(x2, y2);
				temp_vec_paste -= paste_img.at<cv::Vec3b>(x2 + 1, y2);
				vec = norm(temp_vec) > norm(temp_vec_paste) ? temp_vec : temp_vec_paste;

				temp_vec = source_img.at<cv::Vec3b>(x1, y1);
				temp_vec -= source_img.at<cv::Vec3b>(x1 - 1, y1);
				temp_vec_paste = paste_img.at<cv::Vec3b>(x2, y2);
				temp_vec_paste -= paste_img.at<cv::Vec3b>(x2 - 1, y2);
				vec += norm(temp_vec) > norm(temp_vec_paste) ? temp_vec : temp_vec_paste;

				temp_vec = source_img.at<cv::Vec3b>(x1, y1);
				temp_vec -= source_img.at<cv::Vec3b>(x1, y1 + 1);
				temp_vec_paste = paste_img.at<cv::Vec3b>(x2, y2);
				temp_vec_paste -= paste_img.at<cv::Vec3b>(x2, y2 + 1);
				vec += norm(temp_vec) > norm(temp_vec_paste) ? temp_vec : temp_vec_paste;

				temp_vec = source_img.at<cv::Vec3b>(x1, y1);
				temp_vec -= source_img.at<cv::Vec3b>(x1, y1 - 1);
				temp_vec_paste = paste_img.at<cv::Vec3b>(x2, y2);
				temp_vec_paste -= paste_img.at<cv::Vec3b>(x2, y2 - 1);
				vec += norm(temp_vec) > norm(temp_vec_paste) ? temp_vec : temp_vec_paste;

				div_R(figure_inside_index_mat(i, j)) = vec[0];
				div_G(figure_inside_index_mat(i, j)) = vec[1];
				div_B(figure_inside_index_mat(i, j)) = vec[2];

				if (i == 0 || (i > 0 && !figure_inside_mask(i - 1, j)))
				{
					div_R[index] += paste_img.at<cv::Vec3b>(i + paste_point.y() - 1, j + paste_point.x())[0];
					div_G[index] += paste_img.at<cv::Vec3b>(i + paste_point.y() - 1, j + paste_point.x())[1];
					div_B[index] += paste_img.at<cv::Vec3b>(i + paste_point.y() - 1, j + paste_point.x())[2];
				}
				if (i == mask_width - 1 || (i < mask_width - 1 && !figure_inside_mask(i + 1, j)))
				{
					div_R[index] += paste_img.at<cv::Vec3b>(i + paste_point.y() + 1, j + paste_point.x())[0];
					div_G[index] += paste_img.at<cv::Vec3b>(i + paste_point.y() + 1, j + paste_point.x())[1];
					div_B[index] += paste_img.at<cv::Vec3b>(i + paste_point.y() + 1, j + paste_point.x())[2];
				}
				if (j == 0 || (j > 0 && !figure_inside_mask(i, j - 1)))
				{
					div_R[index] += paste_img.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x() - 1)[0];
					div_G[index] += paste_img.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x() - 1)[1];
					div_B[index] += paste_img.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x() - 1)[2];
				}
				if (j == mask_height - 1 || (j < mask_height - 1 && !figure_inside_mask(i, j + 1)))
				{
					div_R[index] += paste_img.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x() + 1)[0];
					div_G[index] += paste_img.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x() + 1)[1];
					div_B[index] += paste_img.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x() + 1)[2];
				}
			}
		}
	}

	Eigen::VectorXf res_R(pixels_cnt), res_G(pixels_cnt), res_B(pixels_cnt);

	res_R = solver.solve(div_R);
	res_G = solver.solve(div_G);
	res_B = solver.solve(div_B);

	for (int i = 0; i < mask_width; i++)
	{
		for (int j = 0; j < mask_height; j++)
		{
			if (figure_inside_mask(i, j) == 1)
			{
				int index = figure_inside_index_mat(i, j);
				int red = res_R(index);
				int green = res_G(index);
				int blue = res_B(index);
				paste_img.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x())[0] = red > 255 ? 255 : (red < 0 ? 0 : red);
				paste_img.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x())[1] = green > 255 ? 255 : (green < 0 ? 0 : green);
				paste_img.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x())[2] = blue > 255 ? 255 : (blue < 0 ? 0 : blue);
			}
		}
	}
}

void ImagePaste::paste_normal(QPoint paste_point, QPoint source_point, cv::Mat& paste_img, cv::Mat& source_img)
{
	for (int i = 0; i < mask_width; i++)
	{
		for (int j = 0; j < mask_height; j++)
		{
			if (figure_inside_mask(i, j) == 1)
			{
				paste_img.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x())[0] = source_img.at<cv::Vec3b>(i + source_point.y(), j + source_point.x())[0];
				paste_img.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x())[1] = source_img.at<cv::Vec3b>(i + source_point.y(), j + source_point.x())[1];
				paste_img.at<cv::Vec3b>(i + paste_point.y(), j + paste_point.x())[2] = source_img.at<cv::Vec3b>(i + source_point.y(), j + source_point.x())[2];
			}
		}
	}
}