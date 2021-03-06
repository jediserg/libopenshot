/**
 * @file
 * @brief Source file for QtTextReader class
 * @author Jonathan Thomas <jonathan@openshot.org>
 *
 * @section LICENSE
 *
 * Copyright (c) 2008-2014 OpenShot Studios, LLC
 * <http://www.openshotstudios.com/>. This file is part of
 * OpenShot Library (libopenshot), an open-source project dedicated to
 * delivering high quality video editing and animation solutions to the
 * world. For more information visit <http://www.openshot.org/>.
 *
 * OpenShot Library (libopenshot) is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * OpenShot Library (libopenshot) is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenShot Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../include/QtTextReader.h"
#include <QImage>
#include <QPainter>

using namespace openshot;

/// Default constructor (blank text)
QtTextReader::QtTextReader() : width(1024), height(768), x_offset(0), y_offset(0), text(""), font("Arial"), size(10.0), text_color("#ffffff"), background_color("#000000"), is_open(false), gravity(GRAVITY_CENTER) {

	// Open and Close the reader, to populate it's attributes (such as height, width, etc...)
	Open();
	Close();
}

QtTextReader::QtTextReader(int width, int height, int x_offset, int y_offset, GravityType gravity, string text, string font, double size, string text_color, string background_color)
: width(width), height(height), x_offset(x_offset), y_offset(y_offset), text(text), font(font), size(size), text_color(text_color), background_color(background_color), is_open(false), gravity(gravity)
{
	// Open and Close the reader, to populate it's attributes (such as height, width, etc...)
	Open();
	Close();
}

// Open reader
void QtTextReader::Open()
{
	// Open reader if not already open
	if (!is_open)
	{
		// create image
		image = std::shared_ptr<QImage>(new QImage(width, height, QImage::Format_RGBA8888));
		image->fill(QColor(background_color.c_str()));
		//start painting
		QPainter painter;
		if(!painter.begin(image.get()))
			return;

		//set background
		painter.setBackground(QBrush(background_color.c_str()));

		//set font color
		painter.setPen(QPen(text_color.c_str()));

		//set font
		painter.setFont(QFont(font.c_str(), size));

		// Set gravity (map between OpenShot and Qt)
		int align_flag = 0;
		switch (gravity)
		{
		case GRAVITY_TOP_LEFT:
			align_flag = Qt::AlignLeft | Qt::AlignTop;
			break;
		case GRAVITY_TOP:
			align_flag = Qt::AlignTop;
			break;
		case GRAVITY_TOP_RIGHT:
			align_flag = Qt::AlignRight | Qt::AlignTop;
			break;
		case GRAVITY_LEFT:
			align_flag = Qt::AlignLeft;
			break;
		case GRAVITY_CENTER:
			align_flag = Qt::AlignCenter;
			break;
		case GRAVITY_RIGHT:
			align_flag = Qt::AlignRight;
			break;
		case GRAVITY_BOTTOM_LEFT:
			align_flag = Qt::AlignLeft | Qt::AlignBottom;
			break;
		case GRAVITY_BOTTOM:
			align_flag = Qt::AlignBottom;
			break;
		case GRAVITY_BOTTOM_RIGHT:
			align_flag = Qt::AlignRight | Qt::AlignBottom;
			break;
		}

		//draw text
		painter.drawText(x_offset, y_offset, width, height, align_flag, text.c_str());

		//end painting
		painter.end();

		// Update image properties
		info.has_audio = false;
		info.has_video = true;
		info.file_size = 0;
		info.vcodec = "Constant image uniform color";
		info.width = width;
		info.height = height;
		info.pixel_ratio.num = 1;
		info.pixel_ratio.den = 1;
		info.duration = 60 * 60 * 24; // 24 hour duration
		info.fps.num = 30;
		info.fps.den = 1;
		info.video_timebase.num = 1;
		info.video_timebase.den = 30;
		info.video_length = round(info.duration * info.fps.ToDouble());

		// Calculate the DAR (display aspect ratio)
		Fraction size(info.width * info.pixel_ratio.num, info.height * info.pixel_ratio.den);

		// Reduce size fraction
		size.Reduce();

		// Set the ratio based on the reduced fraction
		info.display_ratio.num = size.num;
		info.display_ratio.den = size.den;

		// Mark as "open"
		is_open = true;
	}
}

// Close reader
void QtTextReader::Close()
{
	// Close all objects, if reader is 'open'
	if (is_open)
	{
		// Mark as "closed"
		is_open = false;
	}
}

// Get an openshot::Frame object for a specific frame number of this reader.
std::shared_ptr<Frame> QtTextReader::GetFrame(int64_t requested_frame)
{
	if (image)
	{
		// Create or get frame object
		std::shared_ptr<Frame> image_frame(new Frame(requested_frame, image->size().width(), image->size().height(), background_color, 0, 2));

		// Add Image data to frame
		image_frame->AddImage(image);

		// return frame object
		return image_frame;
	} else {
		// return empty frame
		std::shared_ptr<Frame> image_frame(new Frame(1, 640, 480, background_color, 0, 2));

		// return frame object
		return image_frame;
	}

}

// Generate JSON string of this object
string QtTextReader::Json() {

	// Return formatted string
	return JsonValue().toStyledString();
}

// Generate Json::JsonValue for this object
Json::Value QtTextReader::JsonValue() {

	// Create root json object
	Json::Value root = ReaderBase::JsonValue(); // get parent properties
	root["type"] = "QtTextReader";
	root["width"] = width;
	root["height"] = height;
	root["x_offset"] = x_offset;
	root["y_offset"] = y_offset;
	root["text"] = text;
	root["font"] = font;
	root["size"] = size;
	root["text_color"] = text_color;
	root["background_color"] = background_color;
	root["gravity"] = gravity;

	// return JsonValue
	return root;
}

// Load JSON string into this object
void QtTextReader::SetJson(string value) {

	// Parse JSON string into JSON objects
	Json::Value root;
	Json::Reader reader;
	bool success = reader.parse( value, root );
	if (!success)
		// Raise exception
		throw InvalidJSON("JSON could not be parsed (or is invalid)", "");

	try
	{
		// Set all values that match
		SetJsonValue(root);
	}
	catch (exception e)
	{
		// Error parsing JSON (or missing keys)
		throw InvalidJSON("JSON is invalid (missing keys or invalid data types)", "");
	}
}

// Load Json::JsonValue into this object
void QtTextReader::SetJsonValue(Json::Value root) {

	// Set parent data
	ReaderBase::SetJsonValue(root);

	// Set data from Json (if key is found)
	if (!root["width"].isNull())
		width = root["width"].asInt();
	if (!root["height"].isNull())
		height = root["height"].asInt();
	if (!root["x_offset"].isNull())
		x_offset = root["x_offset"].asInt();
	if (!root["y_offset"].isNull())
		y_offset = root["y_offset"].asInt();
	if (!root["text"].isNull())
		text = root["text"].asString();
	if (!root["font"].isNull())
		font = root["font"].asString();
	if (!root["size"].isNull())
		size = root["size"].asDouble();
	if (!root["text_color"].isNull())
		text_color = root["text_color"].asString();
	if (!root["background_color"].isNull())
		background_color = root["background_color"].asString();
	if (!root["gravity"].isNull())
		gravity = (GravityType) root["gravity"].asInt();

	// Re-Open path, and re-init everything (if needed)
	if (is_open)
	{
		Close();
		Open();
	}
}
