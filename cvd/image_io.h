/*                       
	This file is part of the CVD Library.

	Copyright (C) 2005 The Authors

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef CVD_IMAGE_IO_H
#define CVD_IMAGE_IO_H

#include <cvd/exceptions.h>
#include <cvd/image_convert.h>
#include <cvd/internal/load_and_save.h>
#include <cvd/internal/name_builtin_types.h>
#include <cvd/internal/name_CVD_rgb_types.h>
#include <errno.h>
#include <memory>
#include <string>

namespace CVD
{
	
	namespace Exceptions
	{
		/// %Exceptions specific to image loading and saving
		/// @ingroup gException
		namespace Image_IO
		{
			/// Base class for all Image_IO exceptions
			/// @ingroup gException
			struct All: public CVD::Exceptions::All
			{};

			/// This image type is not supported
			/// @ingroup gException
			struct UnsupportedImageType: public All
			{
				UnsupportedImageType();
			};

			/// The file ended before the image
			/// @ingroup gException
			struct EofBeforeImage: public All
			{
				EofBeforeImage();
			};

			/// The ifstream which the file is being read from is not open
			/// @ingroup gException
			struct IfstreamNotOpen: public All
			{
				IfstreamNotOpen();
			};

			/// The image was incorrect
			/// @ingroup gException
			struct MalformedImage: public All
			{
				MalformedImage(const std::string &); ///< Construct from a message string
			};

			/// The loaded image is not the right size
			/// @ingroup gException
			struct ImageSizeMismatch: public All
			{
				ImageSizeMismatch(const ImageRef& src, const ImageRef& dest); ///< Construct from the two sizes
			};

			/// Error writing the image
			/// @ingroup gException
			struct WriteError: public All
			{
				WriteError(const std::string& err); ///< Construct from a message string
			};

			/// Cannot seek in this stream
			/// @ingroup gException
			struct UnseekableIstream: public All
			{
				UnseekableIstream(const std::string& type); ///< Construct from a message string
			};

			/// Type mismatch reading the image (image data is either 8- or 16-bit, and it must be the same in the file)
			/// @ingroup gException
			struct ReadTypeMismatch: public All
			{
				ReadTypeMismatch(bool read8); ///< Constructor is passed <code>true</code> if it was trying to read 8-bit data
			};
			
			/// An error occurred in one of the helper libraries
			/// @ingroup gException
			struct InternalLibraryError: public All
			{
				InternalLibraryError(const std::string& lib, const std::string err); ///< Construct from the library name and the error string
			};

			/// This image subtype is not supported
			/// @ingroup gException
			struct UnsupportedImageSubType: public All
			{
				UnsupportedImageSubType(const std::string &, const std::string&); ///< Construct from a subtype string and an error string
			};

			/// Error in opening file
			/// @ingroup gException
			struct OpenError: public All
			{
				OpenError(const std::string&, int); ///< Construct from the filename and the error number
			};


		}
	}


	////////////////////////////////////////////////////////////////////////////////
	//
	// Image loading
	//

	#ifdef DOXYGEN_INCLUDE_ONLY_FOR_DOCS
	// This is not the real definition, but this is what it would look like if all
	// the macros were expanded. The real definition is in internal/disk_image.h
	/// Contains the enumeration of possible image types
	namespace ImageType
	{
		/// Possible image types
		enum ImageType
		{
			/// PNM image format (PBM, PGM or PPM). This is a raw image format.
			PNM, 
			/// JPEG image format. This is a compressed (lossy) image format, but defaults to 95% quality, which has very few compression artefacts. This image type is only present if libjpeg is available.
			JPEG,
			/// Postscript  format. This outputs a bare PostScript image with the coordinate system set up 
			/// to that (x,y) corresponds to pixel (x,y), with (0,0) being at the top left of the pixel (0,0).
			/// The Y axis is therefore inverted compared to normal postscript drawing, but is image aligned.
			/// To get the drawing axes aligned with the centre of the pixels, write the postscript command
			/// ".5 .5 translate" after the image.
			/// The image data in encoded in ASCII-85 for portability. Helper functions are provided for
			/// generating EPS figures. See CVD::output_eps_header and CVD::output_eps_footer
			PS,
			/// EPS format. This outputs a complete EPS (Encapsulated PostScript) figure.
			EPS,
		};
	}
	#endif
	
	/// Load an image from a stream. This function resizes the Image as necessary.
	/// It will also perform image type conversion (e.g. colour to greyscale)
	/// according the Pixel:::CIE conversion.
	/// @param I The pixel type of the image
	/// @param im The image to receive the loaded image data
	/// @param i The stream
	/// @ingroup gImageIO
	template<class I> void img_load(Image<I>& im, std::istream& i)
	{
		img_load(im, i, Pixel::CIE);
	}

	//  syg21
	template<class I> void img_load(Image<I> &im, const std::string &s)
	{
		std::ifstream i(s.c_str());

		if(!i.good())
			throw Exceptions::Image_IO::OpenError(s, errno);
		img_load(im, i);
	}	

	/// Load an image from a stream into a Basic Image. The function checks that the
	/// BasicImage is the right size, and if not will throw an Image_IO::ImageSizeMismatch
	/// exception.
	/// It will also perform image type conversion (e.g. colour to greyscale)
	/// according the Pixel:::CIE conversion.
	/// @param I The pixel type of the image
	/// @param im The image to receive the loaded image data
	/// @param i The stream
	/// @ingroup gImageIO
	template<class I> void img_load(BasicImage<I>& im, std::istream& i)
	{
		img_load(im, i, Pixel::CIE);
	}

	/// Load an image from a stream. This function resizes the Image as necessary.
	/// It will also perform image type conversion (e.g. colour to greyscale)
	/// according to the conversion specified. See Pixel for a list of common
	/// conversion operations
	/// @param PixelType The pixel type of the image
	/// @param Conversion The conversion class to use
	/// @param im The image to receive the loaded image data
	/// @param i The stream
	/// @param cv The instance of the conversion to use (see Pixel)
	/// @ingroup gImageIO
	template<class PixelType, class Conversion> 
	void img_load(Image<PixelType>&im, std::istream& i, Conversion cv)
	{
		//Open an image for reading (put it in an exception-safe container)
		std::auto_ptr<Image_IO::image_in> in(Image_IO::image_factory::in(i));
		
		//Set up the image to be the correct size (and thereby disown any data as well)
		ImageRef size((int)(in->x_size()), (int)(in->y_size()));
		im.resize(size);

		Internal::load_image(*in, im.data(), cv);
	}

	/// Load an image from a stream into a Basic Image. The function checks that the
	/// BasicImage is the right size, and if not will throw an Image_IO::ImageSizeMismatch
	/// exception.
	/// It will also perform image type conversion (e.g. colour to greyscale)
	/// according to the conversion specified. See Pixel for a list of common
	/// conversion operations
	/// @param PixelType The pixel type of the image
	/// @param Conversion The conversion class to use
	/// @param im The image to receive the loaded image data
	/// @param i The stream
	/// @param cv The instance of the conversion to use (see Pixel)
	/// @ingroup gImageIO
	template<class PixelType, class Conversion> 
	void img_load(BasicImage<PixelType>&im, std::istream& i, Conversion cv)
	{
		//Open an image for reading (put it in an exfeption safe container)
		std::auto_ptr<Image_IO::image_in> in(Image_IO::image_factory::in(i));

		ImageRef size((int)(in->x_size()), (int)(in->y_size()));
		
		//Check image size
		if(size != im.size())
			throw CVD::Exceptions::Image_IO::ImageSizeMismatch(im.size(), size);

		Internal::load_image(*in, im.data(), cv);
	}


	////////////////////////////////////////////////////////////////////////////////
	//
	// Image saving
	//
	#ifndef DOXYGEN_IGNORE_INTERNAL
	namespace Internal
	{
		template<class C, int i> struct save_default_
		{
			static const bool use_16bit=1;
		};

		template<class C> struct save_default_<C,1>
		{
			static const bool use_16bit=Pixel::traits<typename Pixel::Component<C>::type>::bits_used > 8;
		};

		template<class C> struct save_default
		{
			static const bool use_16bit = save_default_<C, Pixel::traits<typename Pixel::Component<C>::type>::integral>::use_16bit;
		};
	}
	#endif	



	/// Save an image to a stream. This function will convert types if necessary.
	/// @param PixelType The pixel type of the image
	/// @param Conversion The conversion class to use
	/// @param im The image to save
	/// @param o The stream 
	/// @param t The image file format to use (see ImageType::ImageType for a list of supported formats)
	/// @param cv The image instance conversion to use, if necessary (see Pixel for a list of common conversions)
	/// @param channels dunno
	/// @param use_16bit dunno
	/// @ingroup gImageIO
	template<class PixelType, class Conversion> 
	void img_save(const BasicImage<PixelType>& im, std::ostream& o, ImageType::ImageType t, Conversion& cv, int channels, bool use_16bit)
	{
		std::string comments;
		
		//I tmp;
		comments = "";
		//comments = "Saving image from pixel type of " + PNM::type_name<PixelType>::name();

		std::auto_ptr<Image_IO::image_out> out(Image_IO::image_factory::out(o, im.size().x, im.size().y, t, channels, use_16bit, comments));

		Internal::save_image(*out, im.data(), cv);
	}


	/// Save an image to a stream. This function will convert types if necessary.
	/// @param PixelType The pixel type of the image
	/// @param Conversion The conversion class to use
	/// @param im The image to save
	/// @param o The stream 
	/// @param t The image file format to use (see ImageType::ImageType for a list of supported formats)
	/// @param cv The instance of the image conversion to use, if necessary (see Pixel for a list of common conversions)
	/// @ingroup gImageIO
	template<class PixelType, class Conversion> void img_save(const BasicImage<PixelType>& im, std::ostream& o, ImageType::ImageType t, Conversion& cv)
	{
		img_save(im, o, t, cv, Pixel::Component<PixelType>::count, Internal::save_default<PixelType>::use_16bit);
	}


	/// Save an image to a stream. This function will convert types if necessary, using
	/// the Pixel::CIE conversion
	/// @param PixelType The pixel type of the image
	/// @param im The image to save
	/// @param o The stream 
	/// @param t The image file format to use (see ImageType::ImageType for a list of supported formats)
	/// @ingroup gImageIO
	template<class PixelType> void img_save(const BasicImage<PixelType>& im, std::ostream& o, ImageType::ImageType t)
	{
		img_save(im, o, t, Pixel::CIE);
	}




	////////////////////////////////////////////////////////////////////////////////
	//
	// Legacy pnm_* functions
	//

	/// Save an image to a stream as a PNM. 
	/// @b Deprecated Use img_save() instead, i.e. <code>img_save(im, o, ImageType::PNM);</code>
	/// @param PixelType The pixel type of the image
	/// @param im The image
	/// @param o The stream
	/// @ingroup gImageIO
	template<class PixelType> void pnm_save(const BasicImage<PixelType>& im, std::ostream& o)
	{
		img_save(im, o, ImageType::PNM);
	}

	/// Load a PNM image from a stream.
	/// @b Deprecated Use img_load(BasicImage<I>& im, std::istream& i) instead. This can handle 
	/// and automatically detect other file formats as well.
	/// @param PixelType The pixel type of the image
	/// @param im The image
	/// @param i The stream
	/// @ingroup gImageIO
	template<class PixelType> void pnm_load(BasicImage<PixelType>& im, std::istream& i)
	{
		img_load(im, i);
	}

	/// Load a PNM image from a stream.
	/// @b Deprecated Use img_load(Image<I>& im, std::istream& i) instead. This can handle 
	/// and automatically detect other file formats as well.
	/// @param PixelType The pixel type of the image
	/// @param im The image
	/// @param i The stream
	/// @ingroup gImageIO
	template<class PixelType> void pnm_load(Image<PixelType>& im, std::istream& i)
	{
		img_load(im, i);
	}

	////////////////////////////////////////////////////////////////////////////////
	//
	// Postscript helper functions
	//

	/// Outputs an EPS footer to an ostream
	/// @param o the ostream
	/// @ingroup gImageIO
	void output_eps_footer(std::ostream& o);

	/// Outputs an EPS header to an ostream. Typical use is to output the header,
	/// save a raw PS image, output some other PS (eg annotations) and the output
	/// the EPS footer.
	/// @param o the ostream
	/// @param xs the width of the image
	/// @param ys the height of the image
	/// @ingroup gImageIO
	void output_eps_header(std::ostream& o, int xs, int ys);


	/// Outputs an EPS header to an ostream. Typical use is to output the header,
	/// save a raw PS image, output some other PS (eg annotations) and the output
	/// the EPS footer.
	/// @param o the ostream
	/// @param s size  of the image
	/// @ingroup gImageIO
	void output_eps_header(std::ostream & o, const ImageRef& s);

	/// Outputs an EPS header to an ostream. Typical use is to output the header,
	/// save a raw PS image, output some other PS (eg annotations) and the output
	/// the EPS footer.
	/// @param o the ostream
	/// @param im the image
	/// @ingroup gImageIO
	template<class PixelType> void output_eps_header(std::ostream& o, const BasicImage<PixelType>& im)
	{
		output_eps_header(o, im.size());
	}

}

#endif
