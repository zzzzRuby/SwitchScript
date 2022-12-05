#include "libpng/pngread.c"

int PNGAPI
png_image_begin_read_from_userdata(png_imagep image, void *data, png_rw_ptr read_fn)
{
	if (image != NULL && image->version == PNG_IMAGE_VERSION)
	{
		if (data != NULL)
		{
			if (png_image_read_init(image) != 0)
			{
				png_set_read_fn(image->opaque->png_ptr, data, read_fn);
				image->opaque->owned_file = 0;
				return png_safe_execute(image, png_image_read_header, image);
			}

			else
				return png_image_error(image, strerror(errno));
		}

		else
			return png_image_error(image,
				"png_image_begin_read_from_userdata: invalid argument");
	}

	else if (image != NULL)
		return png_image_error(image,
			"png_image_begin_read_from_userdata: incorrect PNG_IMAGE_VERSION");

	return 0;
}
