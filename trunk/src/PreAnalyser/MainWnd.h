#pragma once
using namespace System;
using namespace System::Windows;
using namespace System::Windows::Controls;
using namespace System::Windows::Controls::Primitives;

class AudioProcessor;
/*
<Canvas Name="canGraph" Background="White"
      Width="300" Height="200"
      VerticalAlignment="Center" HorizontalAlignment="Center"/>
*/

ref class MainWnd : public Window
{
private:
	Canvas ^canGraph;
	ScrollBar ^dataScroll;
	StackPanel ^mainPanel;
	StackPanel ^selectViewPanel;
	RadioButton ^rbtnFFT;
	RadioButton ^rbtnSum;
	Label ^labelInfo;

protected:
	void InitializeComponent() {
		canGraph = gcnew Canvas();
		dataScroll = gcnew ScrollBar();
		mainPanel = gcnew StackPanel();
		selectViewPanel = gcnew StackPanel();
		rbtnFFT = gcnew RadioButton();
		rbtnSum = gcnew RadioButton();
		labelInfo = gcnew Label();

		canGraph->Width = 1000;
		canGraph->Height = 600;
		canGraph->VerticalAlignment = System::Windows::VerticalAlignment::Center;
		canGraph->HorizontalAlignment = System::Windows::HorizontalAlignment::Center;
		canGraph->MouseMove += gcnew System::Windows::Input::MouseEventHandler(this, &MainWnd::canGraph_MouseMove);

		dataScroll->VerticalAlignment = System::Windows::VerticalAlignment::Bottom;
		dataScroll->HorizontalAlignment = System::Windows::HorizontalAlignment::Stretch;
		dataScroll->Orientation = System::Windows::Controls::Orientation::Horizontal;
		dataScroll->SmallChange = 1;
		dataScroll->Scroll += gcnew ScrollEventHandler(this, &MainWnd::dataScroll_Scroll);

		rbtnFFT->GroupName = "selectView";
		rbtnFFT->IsChecked = true;
		rbtnFFT->Content = "FFT";

		rbtnSum->GroupName = "selectView";
		rbtnSum->IsChecked = false;
		rbtnSum->Content = "Sum";

		labelInfo->Content = "empty";

		selectViewPanel->HorizontalAlignment = System::Windows::HorizontalAlignment::Right;
		selectViewPanel->VerticalAlignment = System::Windows::VerticalAlignment::Top;
		selectViewPanel->Width = 300;
		selectViewPanel->Children->Add(this->rbtnFFT);
		selectViewPanel->Children->Add(this->rbtnSum);
		selectViewPanel->Children->Add(this->labelInfo);

		mainPanel->HorizontalAlignment = System::Windows::HorizontalAlignment::Stretch;
		mainPanel->VerticalAlignment = System::Windows::VerticalAlignment::Stretch;
		mainPanel->Children->Add(this->selectViewPanel);
		mainPanel->Children->Add(this->canGraph);
		mainPanel->Children->Add(this->dataScroll);

		this->Title = "Hello World";
		this->AddChild(this->mainPanel);
		this->Loaded += gcnew RoutedEventHandler(this, &MainWnd::onLoad);
	}

private:
	AudioProcessor *ap;

	System::Windows::Shapes::Polyline ^moving;

	void SetupGraph();
	void UpdateInfo(Point p);

public:
	MainWnd();
	~MainWnd();

	void onLoad(System::Object^  sender, RoutedEventArgs^  e);
	void ButtonOnClick(System::Object^  sender, RoutedEventArgs^  e);
	void dataScroll_Scroll(System::Object^  sender, ScrollEventArgs^  e);
	void canGraph_MouseMove(System::Object^  sender, System::Windows::Input::MouseEventArgs^  e);
};