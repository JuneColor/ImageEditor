
// if you donot use VSStudio, remove this MACRO
#define STBI_MSC_SECURE_CRT

// Thanks to the great STB lib @see https://github.com/nothings/stb
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdlib.h>

/**
* image object that deal with load image and basic pixel based operations
* version: 1.0
* date: 2018/6/20
*/
class ImageObject
{
public:
    int m_width, m_height, m_alpha, m_depth;
    unsigned char *m_image_data = NULL;

    ~ImageObject()
    {
        if (this->isInitized())
        {
            this->releasePngImage();
        }
    }

    int getWidth()
    {
        return m_width;
    }

    int getHeight()
    {
        return m_height;
    }

    int getDepth()
    {
        return m_depth;
    }

    int getMemorySize()
    {
        if (NULL == m_image_data)
        {
            return 0;
        }

        return (m_width * m_height * m_depth * sizeof(unsigned char));
    }

    bool isInitized()
    {
        if (NULL == m_image_data)
        {
            return false;
        }

        return true;
    }

    bool setPngPixel(int x, int y, int red, int green, int blue, int alpha)
    {
        if (this->isInitized())
        {
            m_image_data[y * m_depth + x * m_depth] = red;
            m_image_data[y * m_depth + x * m_depth + 1] = green;
            m_image_data[y * m_depth + x * m_depth + 2] = blue;
            if (4 == m_depth)
            {
                m_image_data[y * m_depth + x * m_depth + 3] = alpha;
            }
            return true;
        }
        return false;
    }

    bool loadPngImage(char const *str_file)
    {
        if (NULL == str_file)
        {
            return false;
        }

        if (NULL != m_image_data)
        {
            return false;
        }

        m_image_data = (unsigned char *) stbi_load(str_file, &m_width, &m_height, &m_depth, 0);

        if (NULL == m_image_data)
        {
            return false;
        }

        return true;
    }

    bool writePngImage(char const *str_file)
    {
        if (NULL == str_file)
        {
            return false;
        }

        if (NULL == m_image_data)
        {
            return false;
        }

        if(1 == stbi_write_png(str_file, m_width, m_height, m_depth, m_image_data, m_depth * m_width))
        {
            return true;
        }

        return false;
    }

    bool releasePngImage()
    {
        if (NULL == m_image_data)
        {
            return true;
        }

        STBI_FREE(m_image_data);
        m_image_data = NULL;

        return true;
    }
};

/**
 * ImageEditor derives from ImageObject that apply algorithm on common data
 * that means you should process only one picture for each ImageEditor Object
 * version: 1.0
 * date: 2018/6/30
 */
class ImageEditor : public ImageObject
{
public:
    bool inverseColor()
    {
        if(!(this->isInitized()))
        {
            return false;
        }

        int rx, gx, bx, pixelPosition;
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
		        pixelPosition = y * m_width * m_depth + x * m_depth;
                rx = pixelPosition + 0;
                gx = pixelPosition + 1;
                bx = pixelPosition + 2;
                m_image_data[rx] = 0xFF - m_image_data[rx];
                m_image_data[gx] = 0xFF - m_image_data[gx];
                m_image_data[bx] = 0xFF - m_image_data[bx];
            }
        }

        return true;
    }

    bool fillRectWithColor(int x, int y, int width, int height, int red, int green, int blue, int alpha)
    {
        if (!(this->verifyNonNegativeColorParams(red, green, blue, alpha)))
        {
            return false;
        }

        if ((x + width) > m_width)
        {
            return false;
        }

        if ((y + height) > m_height)
        {
            return false;
        }

        int rx, gx, bx, ax, pixelPosition;
        for (int y_id = y; y_id < (y + height); ++y_id)
        {
	        for (int x_id = x; x_id < (x + width); ++x_id)
	        {
	            pixelPosition = y_id * m_width * m_depth + x_id * m_depth;
                rx = pixelPosition + 0;
                gx = pixelPosition + 1;
                bx = pixelPosition + 2;
                m_image_data[rx] = red;
                m_image_data[gx] = green;
                m_image_data[bx] = blue;

                if (4 == m_depth)
                {
                    ax = pixelPosition + 3;
                    m_image_data[ax] = alpha;
                }
            }
        }
        return true;
    }

    bool fillAllWithColor(int red, int green, int blue, int alpha)
    {
        if (!(this->verifyNonNegativeColorParams(red, green, blue, alpha)))
        {
            return false;
        }

        int rx, gx, bx, ax, pixelPosition;
        for (int x_id = 0; x_id < m_width; ++x_id)
        {
            for (int y_id = 0; y_id < m_height; ++y_id)
            {
	            pixelPosition = y_id * m_width * m_depth + x_id * m_depth;
                rx = pixelPosition + 0;
                gx = pixelPosition + 1;
                bx = pixelPosition + 2;
                m_image_data[rx] = red;
                m_image_data[gx] = green;
                m_image_data[bx] = blue;

                if (4 == m_depth)
                {
                    ax = pixelPosition + 3;
                    m_image_data[ax] = alpha;
                }
            }
        }
        return true;
    }

    bool setAlpha(int alpha)
    {
        if ((0x00 > alpha) || (0xFF < alpha))
        {
            return false;
        }

        if (4 != m_depth)
        {
            return false;
        }

	int arr_id;
	for (arr_id = 3; arr_id < m_width * m_height * m_depth; arr_id += m_depth)
	{
		m_image_data[arr_id] = alpha;
	}

        return true;
    }

	/**
	 * predicting verticalLength pixels and set it to average value
	 */
    bool verticalBlur(int verticalLength)
    {
        if (0 >= verticalLength)
        {
            return false;
        }

        if (!this->isInitized())
        {
            return false;
        }

        if (m_height <= verticalLength)
        {
            return false;
        }

        unsigned char *m_source_image = (unsigned char *) STBI_MALLOC(this->getMemorySize());
        if (NULL == m_source_image)
        {
            return false;
        }

        if (NULL == memcpy(m_source_image, m_image_data, this->getMemorySize()))
        {
            STBI_FREE(m_source_image);
            return false;
        }

        int average_red, average_green, average_blue, average_alpha;
        int x, y, yh, cnt, pixelPosition;
        for (x = 0; x < m_width; ++x)
        {
            for (y = 0; y < (m_height - 1); ++y)
            {
                average_red = 0;
                average_green = 0;
                average_blue = 0;
                average_alpha = 0;
                cnt = 0;
                for (yh = 0; yh < verticalLength; ++yh)
                {
                    if ((m_height <= (yh + y)))
                    {
                        break;
                    }
		            pixelPosition = (yh + y) * m_width * m_depth + x * m_depth;
                    average_red += m_source_image[pixelPosition + 0];
                    average_green += m_source_image[pixelPosition + 1];
                    average_blue += m_source_image[pixelPosition + 2];
                    if (4 == m_depth)
                    {
                        average_alpha += m_source_image[pixelPosition + 3];
                    }
                    ++cnt;
                }

                if (1 < cnt)
                {
                    average_red /= cnt;
                    average_green /= cnt;
                    average_blue /= cnt;
                    average_alpha /= cnt;
                }
                for (yh = 0; yh < verticalLength; ++yh)
                {
                    if ((m_height <= (yh + y)))
                    {
                        break;
                    }
		            pixelPosition = (yh + y) * m_width * m_depth + x * m_depth;
                    m_image_data[pixelPosition + 0] = average_red;
                    m_image_data[pixelPosition + 1] = average_green;
                    m_image_data[pixelPosition + 2] = average_blue;
                    if (4 == m_depth)
                    {
                        m_image_data[pixelPosition + 3] = average_alpha;
                    }
                    ++cnt;
                }
            }
        }

        STBI_FREE(m_source_image);
        return true;
    }

	/**
	 * predicting verticahorizontalLength pixels and set it to average value
	 */
    bool horizontalBlur(int horizontalLength)
    {
        if (0 >= horizontalLength)
        {
            return false;
        }

        if (!this->isInitized())
        {
            return false;
        }

        if (m_width <= horizontalLength)
        {
            return false;
        }

        unsigned char *m_source_image = (unsigned char *) STBI_MALLOC(this->getMemorySize());
        if (NULL == m_source_image)
        {
            return false;
        }

        if (NULL == memcpy(m_source_image, m_image_data, this->getMemorySize()))
        {
            STBI_FREE(m_source_image);
            return false;
        }

        int average_red, average_green, average_blue, average_alpha;
        int x, y, xh, cnt, pixelPosition;
        for (y = 0; y < m_height; ++y)
        {
            for (x = 0; x < (m_width - 1); ++x)
            {
                average_red = 0;
                average_green = 0;
                average_blue = 0;
                average_alpha = 0;
                cnt = 0;
                for (xh = 0; xh < horizontalLength; ++xh)
                {
                    if ((m_width <= (xh + x)))
                    {
                        break;
                    }
		            pixelPosition = y * m_width * m_depth + (x + xh) * m_depth;
                    average_red += m_source_image[pixelPosition + 0];
                    average_green += m_source_image[pixelPosition + 1];
                    average_blue += m_source_image[pixelPosition + 2];
                    if (4 == m_depth)
                    {
                        average_alpha += m_source_image[pixelPosition + 3];
                    }
                    ++cnt;
                }

                if (1 < cnt)
                {
                    average_red /= cnt;
                    average_green /= cnt;
                    average_blue /= cnt;
                    average_alpha /= cnt;
                }

                for (xh = 0; xh < horizontalLength; ++xh)
                {
                    if ((m_width <= (xh + x)))
                    {
                        break;
                    }
		            pixelPosition = y * m_width * m_depth + (x + xh) * m_depth;
                    m_image_data[pixelPosition + 0] = average_red;
                    m_image_data[pixelPosition + 1] = average_green;
                    m_image_data[pixelPosition + 2] = average_blue;
                    if (4 == m_depth)
                    {
                        m_image_data[pixelPosition + 3] = average_alpha;
                    }
                    ++cnt;
                }
            }
        }

        STBI_FREE(m_source_image);
        return true;
    }

    /** 
	 * apply gaussian blur function on it
     * radiusLength will cause a (2 * radiusLength + 1) * (2 * radiusLength + 1) Matrix for gaussian calculate
     */
    bool gaussianBlur(int radiusLength, double integrity)
    {
        if (0 >= radiusLength)
        {
            return false;
        }

        if (0.0f >= integrity)
        {
            return false;
        }

        if (!this->isInitized())
        {
            return false;
        }

        unsigned char *m_source_image = (unsigned char *) STBI_MALLOC(this->getMemorySize());
        if (NULL == m_source_image)
        {
            return false;
        }

        if (NULL == memcpy(m_source_image, m_image_data, this->getMemorySize()))
        {
            STBI_FREE(m_source_image);
            return false;
        }

        double *m_core_matrix = (double *) STBI_MALLOC((2 * radiusLength + 1) * (2 * radiusLength + 1) * sizeof(double));
        if (NULL == m_core_matrix)
        {
            STBI_FREE(m_source_image);
            return false;
        }

        int x, y;
        double radius, value;
        double pi = 3.1415926f;

        m_core_matrix[(radiusLength + 1) * (2 * radiusLength + 1) - radiusLength] = 1.0f;
        for (y = 0; y <= radiusLength; ++y)
        {
            for (x = 0; x <= radiusLength; ++x)
            {
                radius = pow((radiusLength + 0.0f - x), 2.0f) + pow((radiusLength + 0.0f - y), 2.0f);
                value = (1.0f / (integrity * sqrt(2 * pi))) * exp(-pow(radius, 2.0f) / (2 * integrity * integrity));
                m_core_matrix[y * (2 * radiusLength + 1) + x] = value;
                m_core_matrix[y * (2 * radiusLength + 1) + 2 * radiusLength - x] = value;
                if (y != radiusLength)
                {
                    m_core_matrix[(2 * radiusLength - y) * (2 * radiusLength + 1) + x] = value;
                    m_core_matrix[(2 * radiusLength - y) * (2 * radiusLength + 1) + 2 * radiusLength - x] = value;
                }
            }
        }

        double m_core_sum = 0.0f;
        for (int id = 0; id < (2 * radiusLength + 1) * (2 * radiusLength + 1); ++id)
        {
            m_core_sum += m_core_matrix[id];
        }

        // Normalize Core Matrix
        for (int y = 0; y < (2 * radiusLength + 1); ++y)
        {
            for (int x = 0; x < (2 * radiusLength + 1); ++x)
            {
                m_core_matrix[y * (radiusLength * 2 + 1) + x] /= m_core_sum;
            }
        }

        // Act on convolution calculate use core
        int average_red, average_green, average_blue, average_alpha;
        for (y = 0; y < m_height; ++y)
        {
            for (x = 0; x < m_width; ++x)
            {
                int cx, cy, tx, ty;
                double vRed, vGreen, vBlue, vAlpha;
                double coreVal, value;
				int pixelPosition = 0;
                value = 0.8f;
                coreVal = 1.0f;
                vRed = 0.0f;
                vGreen = 0.0f;
                vBlue = 0.0f;
                vAlpha = 0.0f;
                for (cy = 0; cy < (2 * radiusLength + 1); ++cy)
                {
                    for (cx = 0; cx < (2 * radiusLength + 1); ++cx)
                    {
                        tx = x + cx - radiusLength;
                        ty = y + cy - radiusLength;
                        if ((0 > tx) || (0 > ty))
                        {
                            continue;
                        }
                        if ((m_width <= tx) || (m_height <= ty))
                        {
                            continue;
                        }
                        coreVal = m_core_matrix[cy * (2 * radiusLength + 1) + cx];
		                pixelPosition = (ty * m_width + tx) * m_depth;
                        vRed += coreVal * m_source_image[pixelPosition + 0];
                        vGreen += coreVal * m_source_image[pixelPosition + 1];
                        vBlue += coreVal * m_source_image[pixelPosition + 2];
                        if (4 == m_depth)
                        {
                            vAlpha += coreVal * m_source_image[(ty * m_width + tx) * m_depth + 3];
                        }
                    }
                }

	        pixelPosition = (y * m_width + x) * m_depth;
                m_image_data[pixelPosition + 0] = (unsigned char) vRed;
                m_image_data[pixelPosition + 1] = (unsigned char) vGreen;
                m_image_data[pixelPosition + 2] = (unsigned char) vBlue;
                if (4 == m_depth)
                {
                    m_image_data[pixelPosition + 3] = (int) vAlpha;
                }
            }
        }

        STBI_FREE(m_core_matrix);
        STBI_FREE(m_source_image);
        return true;
    }

	/** 
	* apply gaussian blur function on Red channel
	*
	* radiusLength will cause a (2 * radiusLength + 1) * (2 * radiusLength + 1) Matrix for gaussian calculate
	*/
	bool gaussianRedBlur(int radiusLength, double integrity)
	{
		if (0 >= radiusLength)
		{
			return false;
		}

		if (0.0f >= integrity)
		{
			return false;
		}

		if (!this->isInitized())
		{
			return false;
		}

		unsigned char *m_source_image = (unsigned char *)STBI_MALLOC(this->getMemorySize());
		if (NULL == m_source_image)
		{
			return false;
		}

		if (NULL == memcpy(m_source_image, m_image_data, this->getMemorySize()))
		{
			STBI_FREE(m_source_image);
			return false;
		}

		double *m_core_matrix = (double *)STBI_MALLOC((2 * radiusLength + 1) * (2 * radiusLength + 1) * sizeof(double));
		if (NULL == m_core_matrix)
		{
			STBI_FREE(m_source_image);
			return false;
		}

		int x, y;
		double radius, value;
		double pi = 3.1415926f;

		m_core_matrix[(radiusLength + 1) * (2 * radiusLength + 1) - radiusLength] = 1.0f;
		for (y = 0; y <= radiusLength; ++y)
		{
			for (x = 0; x <= radiusLength; ++x)
			{
				radius = pow((radiusLength + 0.0f - x), 2.0f) + pow((radiusLength + 0.0f - y), 2.0f);
				value = (1.0f / (integrity * sqrt(2 * pi))) * exp(-pow(radius, 2.0f) / (2 * integrity * integrity));
				m_core_matrix[y * (2 * radiusLength + 1) + x] = value;
				m_core_matrix[y * (2 * radiusLength + 1) + 2 * radiusLength - x] = value;
				if (y != radiusLength)
				{
					m_core_matrix[(2 * radiusLength - y) * (2 * radiusLength + 1) + x] = value;
					m_core_matrix[(2 * radiusLength - y) * (2 * radiusLength + 1) + 2 * radiusLength - x] = value;
				}
			}
		}

		double m_core_sum = 0.0f;
		for (int id = 0; id < (2 * radiusLength + 1) * (2 * radiusLength + 1); ++id)
		{
			m_core_sum += m_core_matrix[id];
		}

		// Normalize Matrix
		for (int y = 0; y < (2 * radiusLength + 1); ++y)
		{
			for (int x = 0; x < (2 * radiusLength + 1); ++x)
			{
				m_core_matrix[y * (radiusLength * 2 + 1) + x] /= m_core_sum;
			}
		}

		// Act on convolution calculate use core
		int average_red, average_green, average_blue, average_alpha;
		for (y = 0; y < m_height; ++y)
		{
			for (x = 0; x < m_width; ++x)
			{
				int cx, cy, tx, ty;
				double vRed, vGreen, vBlue, vAlpha;
				double coreVal, value;
				int pixelPosition = 0;
				value = 0.8f;
				coreVal = 1.0f;
				vRed = 0.0f;
				vGreen = 0.0f;
				vBlue = 0.0f;
				vAlpha = 0.0f;
				for (cy = 0; cy < (2 * radiusLength + 1); ++cy)
				{
					for (cx = 0; cx < (2 * radiusLength + 1); ++cx)
					{
						tx = x + cx - radiusLength;
						ty = y + cy - radiusLength;
						if ((0 > tx) || (0 > ty))
						{
							continue;
						}
						if ((m_width <= tx) || (m_height <= ty))
						{
							continue;
						}
						coreVal = m_core_matrix[cy * (2 * radiusLength + 1) + cx];
						pixelPosition = (ty * m_width + tx) * m_depth;
						vRed += coreVal * m_source_image[pixelPosition + 0];
					}
				}

				pixelPosition = (y * m_width + x) * m_depth;
				m_image_data[pixelPosition + 0] = (unsigned char)vRed;
			}
		}

		STBI_FREE(m_core_matrix);
		STBI_FREE(m_source_image);
		return true;
	}

	/**
	* apply gaussian blur function on Green channel
	*
	* radiusLength will cause a (2 * radiusLength + 1) * (2 * radiusLength + 1) Matrix for gaussian calculate
	*/
	bool gaussianGreenBlur(int radiusLength, double integrity)
	{
		if (0 >= radiusLength)
		{
			return false;
		}

		if (0.0f >= integrity)
		{
			return false;
		}

		if (!this->isInitized())
		{
			return false;
		}

		unsigned char *m_source_image = (unsigned char *)STBI_MALLOC(this->getMemorySize());
		if (NULL == m_source_image)
		{
			return false;
		}

		if (NULL == memcpy(m_source_image, m_image_data, this->getMemorySize()))
		{
			STBI_FREE(m_source_image);
			return false;
		}

		double *m_core_matrix = (double *)STBI_MALLOC((2 * radiusLength + 1) * (2 * radiusLength + 1) * sizeof(double));
		if (NULL == m_core_matrix)
		{
			STBI_FREE(m_source_image);
			return false;
		}

		int x, y;
		double radius, value;
		double pi = 3.1415926f;

		m_core_matrix[(radiusLength + 1) * (2 * radiusLength + 1) - radiusLength] = 1.0f;
		for (y = 0; y <= radiusLength; ++y)
		{
			for (x = 0; x <= radiusLength; ++x)
			{
				radius = pow((radiusLength + 0.0f - x), 2.0f) + pow((radiusLength + 0.0f - y), 2.0f);
				value = (1.0f / (integrity * sqrt(2 * pi))) * exp(-pow(radius, 2.0f) / (2 * integrity * integrity));
				m_core_matrix[y * (2 * radiusLength + 1) + x] = value;
				m_core_matrix[y * (2 * radiusLength + 1) + 2 * radiusLength - x] = value;
				if (y != radiusLength)
				{
					m_core_matrix[(2 * radiusLength - y) * (2 * radiusLength + 1) + x] = value;
					m_core_matrix[(2 * radiusLength - y) * (2 * radiusLength + 1) + 2 * radiusLength - x] = value;
				}
			}
		}

		double m_core_sum = 0.0f;
		for (int id = 0; id < (2 * radiusLength + 1) * (2 * radiusLength + 1); ++id)
		{
			m_core_sum += m_core_matrix[id];
		}

		// Normalize Matrix
		for (int y = 0; y < (2 * radiusLength + 1); ++y)
		{
			for (int x = 0; x < (2 * radiusLength + 1); ++x)
			{
				m_core_matrix[y * (radiusLength * 2 + 1) + x] /= m_core_sum;
			}
		}

		// Act on convolution calculate use core
		int average_red, average_green, average_blue, average_alpha;
		for (y = 0; y < m_height; ++y)
		{
			for (x = 0; x < m_width; ++x)
			{
				int cx, cy, tx, ty;
				double vRed, vGreen, vBlue, vAlpha;
				double coreVal, value;
				int pixelPosition = 0;
				value = 0.8f;
				coreVal = 1.0f;
				vRed = 0.0f;
				vGreen = 0.0f;
				vBlue = 0.0f;
				vAlpha = 0.0f;
				for (cy = 0; cy < (2 * radiusLength + 1); ++cy)
				{
					for (cx = 0; cx < (2 * radiusLength + 1); ++cx)
					{
						tx = x + cx - radiusLength;
						ty = y + cy - radiusLength;
						if ((0 > tx) || (0 > ty))
						{
							continue;
						}
						if ((m_width <= tx) || (m_height <= ty))
						{
							continue;
						}
						coreVal = m_core_matrix[cy * (2 * radiusLength + 1) + cx];
						pixelPosition = (ty * m_width + tx) * m_depth;
						vGreen += coreVal * m_source_image[pixelPosition + 1];
					}
				}

				pixelPosition = (y * m_width + x) * m_depth;
				m_image_data[pixelPosition + 1] = (unsigned char)vGreen;
			}
		}

		STBI_FREE(m_core_matrix);
		STBI_FREE(m_source_image);
		return true;
	}

	/** 
	* apply gaussian blur function on Blue channel
	* radiusLength will cause a (2 * radiusLength + 1) * (2 * radiusLength + 1) Matrix for gaussian calculate
	*/
	bool gaussianBlueBlur(int radiusLength, double integrity)
	{
		if (0 >= radiusLength)
		{
			return false;
		}

		if (0.0f >= integrity)
		{
			return false;
		}

		if (!this->isInitized())
		{
			return false;
		}

		unsigned char *m_source_image = (unsigned char *)STBI_MALLOC(this->getMemorySize());
		if (NULL == m_source_image)
		{
			return false;
		}

		if (NULL == memcpy(m_source_image, m_image_data, this->getMemorySize()))
		{
			STBI_FREE(m_source_image);
			return false;
		}

		double *m_core_matrix = (double *)STBI_MALLOC((2 * radiusLength + 1) * (2 * radiusLength + 1) * sizeof(double));
		if (NULL == m_core_matrix)
		{
			STBI_FREE(m_source_image);
			return false;
		}

		int x, y;
		double radius, value;
		double pi = 3.1415926f;
		m_core_matrix[(radiusLength + 1) * (2 * radiusLength + 1) - radiusLength] = 1.0f;
		for (y = 0; y <= radiusLength; ++y)
		{
			for (x = 0; x <= radiusLength; ++x)
			{
				radius = pow((radiusLength + 0.0f - x), 2.0f) + pow((radiusLength + 0.0f - y), 2.0f);
				value = (1.0f / (integrity * sqrt(2 * pi))) * exp(-pow(radius, 2.0f) / (2 * integrity * integrity));
				m_core_matrix[y * (2 * radiusLength + 1) + x] = value;
				m_core_matrix[y * (2 * radiusLength + 1) + 2 * radiusLength - x] = value;
				if (y != radiusLength)
				{
					m_core_matrix[(2 * radiusLength - y) * (2 * radiusLength + 1) + x] = value;
					m_core_matrix[(2 * radiusLength - y) * (2 * radiusLength + 1) + 2 * radiusLength - x] = value;
				}
			}
		}

		double m_core_sum = 0.0f;
		for (int id = 0; id < (2 * radiusLength + 1) * (2 * radiusLength + 1); ++id)
		{
			m_core_sum += m_core_matrix[id];
		}

		// Normalize Matrix
		for (int y = 0; y < (2 * radiusLength + 1); ++y)
		{
			for (int x = 0; x < (2 * radiusLength + 1); ++x)
			{
				m_core_matrix[y * (radiusLength * 2 + 1) + x] /= m_core_sum;
			}
		}

		// Act on convolution calculate use core
		int average_red, average_green, average_blue, average_alpha;
		for (y = 0; y < m_height; ++y)
		{
			for (x = 0; x < m_width; ++x)
			{
				int cx, cy, tx, ty;
				double vRed, vGreen, vBlue, vAlpha;
				double coreVal, value;
				int pixelPosition = 0;
				value = 0.8f;
				coreVal = 1.0f;
				vRed = 0.0f;
				vGreen = 0.0f;
				vBlue = 0.0f;
				vAlpha = 0.0f;
				for (cy = 0; cy < (2 * radiusLength + 1); ++cy)
				{
					for (cx = 0; cx < (2 * radiusLength + 1); ++cx)
					{
						tx = x + cx - radiusLength;
						ty = y + cy - radiusLength;
						if ((0 > tx) || (0 > ty))
						{
							continue;
						}
						if ((m_width <= tx) || (m_height <= ty))
						{
							continue;
						}
						coreVal = m_core_matrix[cy * (2 * radiusLength + 1) + cx];
						pixelPosition = (ty * m_width + tx) * m_depth;
						vBlue += coreVal * m_source_image[pixelPosition + 2];
					}
				}

				pixelPosition = (y * m_width + x) * m_depth;
				m_image_data[pixelPosition + 2] = (unsigned char)vBlue;
			}
		}

		STBI_FREE(m_core_matrix);
		STBI_FREE(m_source_image);
		return true;
	}

	/** 
	* transform into gray color
	* that is R = G = B = Gray
	* way1: use Gray = (R*38 + G*75 + B*15) >> 7
	* way2: use Gray = (R*30 + G*59 + B*11 + 50) / 100
	* way3: use Gray = R*0.299 + G*0.587 + B*0.114
	*/
	bool transformToGray()
	{
		if (!this->isInitized())
		{
			return false;
		}

		float grayVal;
		int x, y, pixelPosition;
		for (y = 0; y < m_height; ++y)
		{
			for (x = 0; x < m_width; ++x)
			{
				pixelPosition = (y * m_width + x) * m_depth;
				grayVal = m_image_data[pixelPosition + 0] * 0.299;
				grayVal += m_image_data[pixelPosition + 1] * 0.587;
				grayVal +=   m_image_data[pixelPosition + 2] * 0.114;
				m_image_data[pixelPosition + 0] = (unsigned char) grayVal;
				m_image_data[pixelPosition + 1] = (unsigned char) grayVal;
				m_image_data[pixelPosition + 2] = (unsigned char) grayVal;
			}
		}

		return true;
	}

	/*
	* decay R/G/B with decayCoeff
	*/
	bool decayColor(float decayCoeff)
	{
		if (!this->isInitized())
		{
			return false;
		}

		if ((0.0f > decayCoeff) || (1.0f <= decayCoeff))
		{
			return false;
		}

		int x, y, pixelPosition;
		for (y = 0; y < m_height; ++y)
		{
			for (x = 0; x < m_width; ++x)
			{
				pixelPosition = (y * m_width + x) * m_depth;
				m_image_data[pixelPosition + 0] *= decayCoeff;
				m_image_data[pixelPosition + 1] *= decayCoeff;
				m_image_data[pixelPosition + 2] *= decayCoeff;
			}
		}

		return true;
	}

	/*
	* decay R/G/B with coeffRed, coeffGreen, coeffBlue
	*/
	bool decayRGB(float coeffRed, float coeffGreen, float coeffBlue)
	{
		if (!this->isInitized())
		{
			return false;
		}

		if (0.0f > coeffRed)
		{
			return false;
		}

		if (0.0f > coeffGreen)
		{
			return false;
		}

		if (0.0f > coeffBlue)
		{
			return false;
		}

		int x, y, pixelPosition;
		for (y = 0; y < m_height; ++y)
		{
			for (x = 0; x < m_width; ++x)
			{
				pixelPosition = (y * m_width + x) * m_depth;
				m_image_data[pixelPosition + 0] *= coeffRed;
				m_image_data[pixelPosition + 1] *= coeffGreen;
				m_image_data[pixelPosition + 2] *= coeffBlue;
			}
		}

		return true;
	}

	/*
	* transform image to {0x00 | 0xFF} mode with given threshold
	* alpha channel will be ignored for keep transparency
	* Range: val < threshold = 0x00; val >= threshold = 0xFF
	*/
	bool binaryTransform(int redThreshold, int greenThreshold, int blueThreshold)
	{
		if (!this->isInitized())
		{
			return false;
		}

		if ((0x00 > redThreshold) || (0xFF < redThreshold))
		{
			return false;
		}

		if ((0x00 > greenThreshold) || (0xFF < greenThreshold))
		{
			return false;
		}

		if ((0x00 > blueThreshold) || (0xFF < blueThreshold))
		{
			return false;
		}
		
		int x, y, pixelPosition;
		for (y = 0; y < m_height; ++y)
		{
			for (x = 0; x < m_width; ++x)
			{
				pixelPosition = (y * m_width + x) * m_depth;
				m_image_data[pixelPosition + 0] = (m_image_data[pixelPosition + 0] > redThreshold) ? 0x00 : 0xFF;
				m_image_data[pixelPosition + 1] = (m_image_data[pixelPosition + 1] > greenThreshold) ? 0x00 : 0xFF;
				m_image_data[pixelPosition + 2] = (m_image_data[pixelPosition + 2] > blueThreshold) ? 0x00 : 0xFF;
			}
		}

		return true;
	}

private:
    bool verifyNonNegativeColorParams(int red, int green, int blue, int alpha)
    {
        if ((0 > red)
            || (0 > green)
            || (0 > blue)
            || (0 > alpha))
        {
            return false;
        }

        return true;
    }

    bool printfBoxCore(double *core, int coreSize, const char *title)
    {
        if (NULL == core)
        {
            return false;
        }

        if (0 >= coreSize)
        {
            return false;
        }

        double m_core_sum = 0.0f;
        printf("\n%s", title);
        printf("\nCoreSize=%d\n", coreSize);
        for (int j = 0; j < (2 * coreSize + 1); ++j)
        {
            for (int i = 0; i < (2 * coreSize + 1); ++i)
            {
                printf("%5.2f\t", core[j * (coreSize * 2 + 1) + i]);
                m_core_sum += core[j * (coreSize * 2 + 1) + i];
            }
            printf("\n");
        }
        printf("CoreSum=%f", m_core_sum);
    }
};

/**
 * The following main() is used to test functions above
 */
int main()
{
    const char *SRC_FILE = "source.png";
    const char *TARGET_FILE = "process_result.png";

    ImageEditor imageObj;
    imageObj.loadPngImage(SRC_FILE);

    if (false == imageObj.isInitized())
    {
        printf("Object not initialized!\n");
    }
    else
    {
        printf("Object Load success\n");
    }

    printf("Object args w=%d, h=%d, n=%d\n", imageObj.getWidth(), imageObj.getHeight(), imageObj.getDepth());

    //if (false == imageObj.inverseColor())
    //{
    //    printf("Object inverse color failed!\n");
    //}
    //else
    //{
    //    printf("Object inverse color success\n");
    //}

    //if (false == imageObj.fillAllWithColor(0xCC, 0xFF, 0xCC, 0x00))
    //{
    //    printf("Object fill all color failed!\n");
    //}
    //else
    //{
    //    printf("Object fill all color success\n");
    //}

    //if (false == imageObj.fillRectWithColor(0, 0, 300, 200, 0xCC, 0xFF, 0xCC, 0x00))
    //{
    //    printf("Object fill all color failed!\n");
    //}
    //else
    //{
    //    printf("Object fill all color success\n");
    //}

    //if (false == imageObj.verticalBlur(30))
    //{
    //    printf("Object vertical blur failed!\n");
    //}
    //else
    //{
    //    printf("Object vertical blur success\n");
    //}

    //if (false == imageObj.horizontalBlur(30))
    //{
    //    printf("Object horizontal blur failed!\n");
    //}
    //else
    //{
    //    printf("Object horizontal blur success\n");
    //}

    //if (false == imageObj.gaussianBlur(5, 20.0f))
    //{
    //    printf("Object Gaussian blur failed!\n");
    //}
    //else
    //{
    //    printf("Object Gaussian blur success\n");
    //}
	
	if (false == imageObj.transformToGray())
	{
		printf("Object transform to gray failed!\n");
	}
	else
	{
		printf("Object transform to gray success\n");
	}

	if (false == imageObj.binaryTransform(0x80, 0x80, 0x80))
	{
		printf("Object binary transform failed!\n");
	}
	else
	{
		printf("Object binary transform success\n");
	}

	//if (false == imageObj.decayColor(0.80f))
	//{
	//	printf("Object decay color failed!\n");
	//}
	//else
	//{
	//	printf("Object decay color  success\n");
	//}

	//if (false == imageObj.decayRGB(0.3f, 0.9f, 0.6f))
	//{
	//	printf("Object decay RGB color failed!\n");
	//}
	//else
	//{
	//	printf("Object decay RGB color  success\n");
	//}

	//if (false == imageObj.gaussianRedBlur(10, 10.0f))
	//{
	//    printf("Object Gaussian blur failed!\n");
	//}
	//else
	//{
	//    printf("Object Gaussian blur success\n");
	//}

	//if (false == imageObj.gaussianGreenBlur(10, 10.0f))
	//{
	//    printf("Object Gaussian blur failed!\n");
	//}
	//else
	//{
	//    printf("Object Gaussian blur success\n");
	//}

	//if (false == imageObj.gaussianBlueBlur(10, 10.0f))
	//{
	//    printf("Object Gaussian blur failed!\n");
	//}
	//else
	//{
	//    printf("Object Gaussian blur success\n");
	//}

    //if (false == imageObj.setAlpha(150))
    //{
    //    printf("Object set alpha failed!\n");
    //}
    //else
    //{
    //    printf("Object set alpha success\n");
    //}

    if (false == imageObj.writePngImage(TARGET_FILE))
    {
        printf("Write target file failed!\n");
    }
    else
    {
        printf("Write target file success\n");
    }

    printf("DONE\n");

    return 0;
}
