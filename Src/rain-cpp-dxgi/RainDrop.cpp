#include "RainDrop.h"

#include <wrl/client.h>

Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> RainDrop::DropColorBrush;
std::vector<Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>> RainDrop::PrebuiltSplatterOpacityBrushes;

RainDrop::RainDrop(const int windowWidth,
                   const int windowHeight,
                   const int xVelocity,
                   const RainDropType type):
	VelocityX(static_cast<float>(xVelocity)),
	WindowWidth(windowWidth),
	WindowHeight(windowHeight),
	Type(type)
{
	Initialize();
}

bool RainDrop::DidDropLand() const
{
	return LandedDrop;
}

bool RainDrop::ShouldBeErasedAndDeleted() const
{
	return FramesForSplatter >= MAX_SPLUTTER_FRAME_COUNT_;
}

void RainDrop::MoveToNewPosition()
{
	if (LandedDrop)
	{
		return;
	}

	// Update the position of the raindrop
	Ellipse.point.x += VelocityX;
	Ellipse.point.y += VelocityY;


	// Apply gravity
	if (Type == RainDropType::Splatter)
	{
		VelocityY += Gravity;
	}


	if (Type == RainDropType::MainDrop)
	{
		if (Ellipse.point.y + Ellipse.radiusY >= WindowHeight)
		{
			Ellipse.point.y = static_cast<float>(WindowHeight);
			LandedDrop = true;
			for (int i = 0; i < 3; i++)
			{
				auto splatter = new RainDrop(WindowWidth, WindowHeight, 0, RainDropType::Splatter);
				float xSpeed = static_cast<float>(GetRandomNumber(-1, 3));
				//xSpeed = xSpeed == 0 ? 0.75f : xSpeed;
				float ySpeed = 10 / (xSpeed * 3.0f);

				splatter->UpdatePositionAndSpeed(Ellipse.point.x, Ellipse.point.y, xSpeed, ySpeed);
				Splatters.push_back(splatter);
			}
		}
	}

	// Bounce off the edges for Splatter type
	if (Type == RainDropType::Splatter)
	{
		if (Ellipse.point.x + Ellipse.radiusX > WindowWidth || Ellipse.point.x - Ellipse.radiusX < 0)
		{
			VelocityX = -VelocityX;
		}

		if (Ellipse.point.y + Ellipse.radiusY > WindowHeight)
		{
			Ellipse.point.y = WindowHeight - Ellipse.radiusY; // Keep the ellipse within bounds
			VelocityY = -VelocityY * BounceDamping; // Bounce with damping
			SplashCount++;
		}

		if (Ellipse.point.y - Ellipse.radiusY < 0)
		{
			Ellipse.point.y = Ellipse.radiusY; // Keep the ellipse within bounds
			VelocityY = -VelocityY; // Reverse the direction if it hits the top edge
		}
	}
}

void RainDrop::UpdatePositionAndSpeed(const FLOAT x, const FLOAT y, const float xSpeed, const float ySpeed)
{
	Ellipse.point.x = x;
	Ellipse.point.y = y;
	VelocityX = xSpeed;
	VelocityY = ySpeed;
}

void RainDrop::Draw(ID2D1DeviceContext* dc)
{
	//D2D1_RECT_F rect;
	//rect.top = 0;
	//rect.left = 0;
	//rect.right = window_width_;
	//rect.bottom = window_height_;
	//dc->DrawRectangle(&rect, pBrush);


	if (!LandedDrop && Ellipse.point.y >= 0 && Ellipse.point.x >= 0)
	{
		if (Type == RainDropType::MainDrop)
		{
			//dc->FillEllipse(ellipse, pBrush);
			D2D1_POINT_2F prevPoint;
			prevPoint.x = Ellipse.point.x - 3 * VelocityX;
			prevPoint.y = Ellipse.point.y - 3 * VelocityY;
			dc->DrawLine(prevPoint, Ellipse.point, DropColorBrush.Get());
		}
	}
	if (!Splatters.empty())
	{
		for (const auto drop : Splatters)
		{
			drop->DrawSplatter(dc, PrebuiltSplatterOpacityBrushes[FramesForSplatter].Get());
			drop->MoveToNewPosition();
		}
		FramesForSplatter++;
	}
}

void RainDrop::DrawSplatter(ID2D1DeviceContext* dc, ID2D1SolidColorBrush* pBrush) const
{
	if (!LandedDrop && Ellipse.point.y >= 0 && Ellipse.point.x >= 0)
	{
		if (Type == RainDropType::Splatter)
		{
			if (SplashCount < 3)
			{
				dc->FillEllipse(Ellipse, pBrush);
			}
		}
	}
}


RainDrop::~RainDrop()
{
	for (const auto drop : Splatters)
	{
		delete drop;
	}
}

int RainDrop::GetRandomNumber(int min, int max)
{
	// Ensure that x is less than or equal to y
	if (min > max)
	{
		std::swap(min, max);
	}
	return std::rand() % (max - min + 1) + min;
}

void RainDrop::Initialize()
{
	// Initialize position and size
	Ellipse.point.x = static_cast<float>(GetRandomNumber(-WindowWidth, WindowWidth));

	const int y = (GetRandomNumber(-WindowHeight, 0) / 10) * 10;
	Ellipse.point.y = static_cast<float>(y);

	if (Type == RainDropType::MainDrop)
	{
		Ellipse.radiusX = 1;
		Ellipse.radiusY = 1;
	}
	else
	{
		Ellipse.radiusX = 2.0f;
		Ellipse.radiusY = 2.0f;
	}

	// Initialize velocity and physics parameters
	//velocity_x_ = 3.0f;
	VelocityY = 15.0f;
	Gravity = 0.4f;
	BounceDamping = 1.0f;
}


void RainDrop::SetRainColor(ID2D1DeviceContext* dc, const COLORREF color)
{
	// Extract the individual color components from COLORREF
	float red = static_cast<float>(GetRValue(color)) / 255.0f;
	float green = static_cast<float>(GetGValue(color)) / 255.0f;
	float blue = static_cast<float>(GetBValue(color)) / 255.0f;

	dc->CreateSolidColorBrush(D2D1::ColorF(red, green, blue, 1.0f), DropColorBrush.GetAddressOf());

	if (!PrebuiltSplatterOpacityBrushes.empty())
	{
		PrebuiltSplatterOpacityBrushes.clear();
	}
	for (int i = 0; i < MAX_SPLUTTER_FRAME_COUNT_; i++)
	{
		float alpha = 1.0f - static_cast<float>(i) / static_cast<float>(MAX_SPLUTTER_FRAME_COUNT_);
		const D2D1_COLOR_F splatterColor = D2D1::ColorF(red, green, blue, alpha);
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> splatterColorBrush;
		dc->CreateSolidColorBrush(splatterColor, splatterColorBrush.GetAddressOf());
		PrebuiltSplatterOpacityBrushes.push_back(splatterColorBrush);
	}
}
