#include "MainWnd.h"

#include "AudioProcessor.h"

using namespace System::Windows::Media;
using namespace System::Windows::Shapes;

MainWnd::MainWnd() {
	InitializeComponent();

	ap = new AudioProcessor();
}

MainWnd::~MainWnd() {
	delete ap;
}

void MainWnd::onLoad(System::Object^  sender, RoutedEventArgs^  e) {
	ap->UpdateSum();

	dataScroll->Minimum = 0;
	dataScroll->Maximum = ap->getDataSum().data.size() - 1;

	PointCollection ^points = gcnew PointCollection();
	points->Add(Point(10, 100));
	points->Add(Point(100, 10));

	moving = gcnew Polyline();
	moving->StrokeThickness = 1;
	moving->Stroke = Brushes::Green;
	moving->Points = points;

	SetupGraph();
}

void MainWnd::SetupGraph() {
	canGraph->Children->Clear();

	const double margin = 10;
	double xmin = margin;
	double xmax = canGraph->Width - margin;
	double xsz = xmax - xmin;
	double ymin = margin;
	double ymax = canGraph->Height - margin;
	double ysz = ymax - ymin;
	const double step = 10;

//	canGraph->Background = gcnew SolidColorBrush(Colors::White);

	// Make the X axis.
	GeometryGroup ^xaxis_geom = gcnew GeometryGroup();
	xaxis_geom->Children->Add(gcnew LineGeometry(Point(0, ymax), Point(canGraph->Width, ymax)));
	for (double x = xmin + step; x <= canGraph->Width - step; x += step)
	{
		xaxis_geom->Children->Add(gcnew LineGeometry(Point(x, ymax - margin / 2), Point(x, ymax + margin / 2)));
	}

	Path ^xaxis_path = gcnew Path();
	xaxis_path->StrokeThickness = 1;
	xaxis_path->Stroke = Brushes::Black;
	xaxis_path->Data = xaxis_geom;

	canGraph->Children->Add(xaxis_path);

	// Make the Y ayis.
	GeometryGroup ^yaxis_geom = gcnew GeometryGroup();
	yaxis_geom->Children->Add(gcnew LineGeometry(Point(xmin, 0), Point(xmin, canGraph->Height)));
	for (double y = step; y <= canGraph->Height - step; y += step)
	{
		yaxis_geom->Children->Add(gcnew LineGeometry(Point(xmin - margin / 2, y), Point(xmin + margin / 2, y)));
	}

	Path ^yaxis_path = gcnew Path();
	yaxis_path->StrokeThickness = 1;
	yaxis_path->Stroke = Brushes::Black;
	yaxis_path->Data = yaxis_geom;

	canGraph->Children->Add(yaxis_path);

	ViewData &data = ap->getDataSum();
	float id = 0;
	PointCollection ^points = gcnew PointCollection();
	for (float v : data.data[dataScroll->Track->Value]) {
		float val_loc = v / data.max_val;
		float x_loc = id / data.data[dataScroll->Track->Value].size();

		points->Add(Point(xmin + (xsz * x_loc), ymax - (val_loc * ysz)));
		id += 1;
	}

	Polyline ^polyline = gcnew Polyline();
	polyline->StrokeThickness = 1;
	polyline->Stroke = Brushes::Red;
	polyline->Points = points;

	canGraph->Children->Add(polyline);

	canGraph->Children->Add(moving);
}

void MainWnd::UpdateInfo(Point p) {
	const double margin = 10;
	double xmin = margin;
	double xmax = canGraph->Width - margin;
	double xsz = xmax - xmin;
	double ymin = margin;
	double ymax = canGraph->Height - margin;
	double ysz = ymax - ymin;
	const double step = 10;

	ViewData &data = ap->getDataSum();
	int dataxlen = data.data[dataScroll->Track->Value].size();
	float dataylen = data.max_val;

	double r_x = ((p.X - xmin) / xsz) * dataxlen * data.xmul;
	double r_y = dataylen - ((p.Y - ymin) / ysz) * dataylen;

	labelInfo->Content = "X : " + r_x.ToString() + " Y : " + r_y.ToString();
}

void MainWnd::ButtonOnClick(System::Object^  sender, RoutedEventArgs^  e) {
	this->Close();
}

void MainWnd::dataScroll_Scroll(System::Object^  sender, ScrollEventArgs^  e) {
	SetupGraph();
}

void MainWnd::canGraph_MouseMove(System::Object^  sender, System::Windows::Input::MouseEventArgs^  e) {
	Canvas^ b = safe_cast<Canvas^>(sender);
	Point p = System::Windows::Input::Mouse::GetPosition(b);

	PointCollection ^points = gcnew PointCollection();
	points->Add(Point(p.X, p.Y));
	points->Add(Point(p.X, 0));

	moving->Points = points;

	UpdateInfo(p);
}