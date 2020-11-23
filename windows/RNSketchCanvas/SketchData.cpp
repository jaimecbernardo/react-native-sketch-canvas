#include "pch.h"
#include "SketchData.h"
#include "Utility.h"

namespace winrt
{
  using namespace Windows::Foundation;
  using namespace Windows::UI;
  using namespace Windows::UI::Xaml;
  using namespace Windows::UI::Xaml::Shapes;
  using namespace Windows::UI::Xaml::Controls;
  using namespace Windows::UI::Xaml::Media;
}

namespace winrt::RNSketchCanvas::implementation
{
  Point SketchData::midPoint(const Point& p1, const Point& p2)
  {
    return Point((p1.X + p2.X) * .5f, (p1.Y + p2.Y) * .5f);
  }

  SketchData::SketchData(Canvas canvas, int id, Color strokeColor, float strokeWidth)
  {
    this->id = id;
    this->strokeColor = strokeColor;
    this->strokeWidth = strokeWidth;
    this->isTranslucent = true; // ????? strokeColor.A != 1 && !Utility::isSameColor(strokeColor, Colors::Transparent());
    mPath = this->isTranslucent ? winrt::Windows::UI::Xaml::Shapes::Path() : nullptr;
    if (mPath != nullptr)
    {
      PathGeometry pathGeometry;
      mPath.value().Data(pathGeometry);
      PathFigureCollection pathFigureCollection;
      pathGeometry.Figures(pathFigureCollection);
      mPath.value().Stroke(SolidColorBrush(strokeColor));
      mPath.value().StrokeThickness(strokeWidth);
      mPath.value().StrokeStartLineCap(PenLineCap::Round);
      mPath.value().StrokeEndLineCap(PenLineCap::Round);
      mPath.value().StrokeLineJoin(PenLineJoin::Round);
      canvas.Children().Append(mPath.value());
    }
  }

  SketchData::SketchData(Canvas canvas, int id, Color strokeColor, float strokeWidth, std::vector<Point> points)
  {
    this->id = id;
    this->strokeColor = strokeColor;
    this->strokeWidth = strokeWidth;
    this->points.insert(std::end(this->points), std::begin(points), std::end(points));
    this->isTranslucent = true; // ????? strokeColor.A != 1 && !Utility::isSameColor(strokeColor, Colors::Transparent());
    mPath = this->isTranslucent ? evaluatePath() : nullptr;
    if (mPath != nullptr)
    {
      canvas.Children().Append(mPath.value());
    }
  }

  Rect SketchData::addPoint(winrt::Windows::Foundation::Point& p)
  {
    points.push_back(p);
    Rect updateRect = Rect();
    int pointsCount = points.size();
    if (isTranslucent)
    {
      if (pointsCount >= 3)
      {
        addPointToPath(mPath.value(),
          points[pointsCount - 3],
          points[pointsCount - 2],
          p);
      } else if (pointsCount >= 2)
      {
        addPointToPath(mPath.value(), points[0], points[0], p);
      } else
      {
        addPointToPath(mPath.value(), p, p, p);
      }

      float x = p.X, y = p.Y;
      if (!mDirty.has_value())
      {
        mDirty = Rect(x, y, 1, 1);
        updateRect = Rect(x - strokeWidth, y - strokeWidth, strokeWidth * 2, strokeWidth * 2);
      } else
      {
        mDirty = RectHelper::Union(mDirty.value(), p);
        updateRect =
          Rect(mDirty.value().X - strokeWidth,
            mDirty.value().Y - strokeWidth,
            mDirty.value().Width + strokeWidth * 2,
            mDirty.value().Height + strokeWidth * 2
          );
      }
    } else
    {
      if (pointsCount >= 3)
      {
        Point a = points[pointsCount - 3];
        Point b = points[pointsCount - 2];
        Point c = p;
        Point prevMid = midPoint(a, b);
        Point currentMid = midPoint(b, c);
        
        updateRect = Rect(prevMid.X, prevMid.Y, 0, 0);
        updateRect = RectHelper::Union(updateRect, b);
        updateRect = RectHelper::Union(updateRect, currentMid);
      } else if (pointsCount >= 2)
      {
        Point a = points[pointsCount - 2];
        Point b = p;
        Point mid = midPoint(a, b);
        updateRect = Rect(a.X, a.Y, 0, 0);
        updateRect = RectHelper::Union(updateRect, mid);
      } else
      {
        updateRect = Rect(p.X, p.Y, 0, 0);
      }
      updateRect = Rect(
        updateRect.X - strokeWidth,
        updateRect.Y - strokeWidth,
        updateRect.Width + strokeWidth * 2,
        updateRect.Height + strokeWidth * 2
      );
    }
    // roundout?
    return updateRect;
  }

  void SketchData::drawLastPoint(winrt::Windows::UI::Xaml::Controls::Canvas canvas)
  {
    int pointsCount = points.size();
    if (pointsCount < 1)
    {
      return;
    }
    draw(canvas, pointsCount - 1);
  }

  void SketchData::draw(winrt::Windows::UI::Xaml::Controls::Canvas canvas)
  {
    if (this->isTranslucent)
    {
      canvas.UpdateLayout();
    } else
    {
      int pointsCount = points.size();
      for (int i = 0; i < pointsCount; i++)
      {
        draw(canvas, i);
      }
    }
  }

  void SketchData::draw(winrt::Windows::UI::Xaml::Controls::Canvas canvas, int pointIndex)
  {
    canvas.UpdateLayout();
  }

  void SketchData::removeFromCanvas(winrt::Windows::UI::Xaml::Controls::Canvas canvas)
  {
    int index = -1;
    for (int i = 0; i < canvas.Children().Size(); i++)
    {
      if (canvas.Children().GetAt(i) == mPath.value())
      {
        index = i;
        break;
      }
    }
    if (index > -1)
    {
      canvas.Children().RemoveAt(index);
    }
  }

  Path SketchData::evaluatePath()
  {
    int pointsCount = points.size();
    Path path;
    PathGeometry pathGeometry;
    path.Data(pathGeometry);
    PathFigureCollection pathFigureCollection;
    pathGeometry.Figures(pathFigureCollection);
    path.Stroke(SolidColorBrush(strokeColor));
    path.StrokeThickness(strokeWidth);
    path.StrokeStartLineCap(PenLineCap::Round);
    path.StrokeEndLineCap(PenLineCap::Round);
    path.StrokeLineJoin(PenLineJoin::Round);

    for (int pointIndex = 0; pointIndex < pointsCount; pointIndex++)
    {
      if (pointsCount >= 3 && pointIndex >= 2)
      {
        Point a = points[pointIndex - 2];
        Point b = points[pointIndex - 1];
        Point c = points[pointIndex];
        Point prevMid = midPoint(a, b);
        Point currMid = midPoint(b, c);
        
        // Draw a curve
        PathFigure pathFigure;
        pathFigure.StartPoint(prevMid);
        pathFigureCollection.Append(pathFigure);
        PathSegmentCollection segmentCollection;
        QuadraticBezierSegment segment;
        segment.Point1(b);
        segment.Point2(currMid);
        segmentCollection.Append(segment);
        pathFigure.Segments(segmentCollection);
      } else if (pointsCount >= 2 && pointIndex >= 1)
      {
        Point a = points[pointIndex - 1];
        Point b = points[pointIndex];
        Point mid = midPoint(a, b);
        
        // Draw a line to the middle of points a and b
        // This is so the next draw which uses a curve looks correct and continues from there
        PathFigure pathFigure;
        pathFigure.StartPoint(a);
        pathFigureCollection.Append(pathFigure);
        PathSegmentCollection segmentCollection;
        LineSegment segment;
        segment.Point(mid);
        segmentCollection.Append(segment);
        pathFigure.Segments(segmentCollection);
      } else if (pointsCount >= 1)
      {
        Point a = points[pointIndex];

        // Draw a single point
        PathFigure pathFigure;
        pathFigure.StartPoint(a);
        pathFigureCollection.Append(pathFigure);
        PathSegmentCollection segmentCollection;
        LineSegment segment;
        segment.Point(a);
        segmentCollection.Append(segment);
        pathFigure.Segments(segmentCollection);
      }
    }

    return path;
  }

  void SketchData::addPointToPath(
    Path& path,
    const Point& tPoint,
    const Point& pPoint,
    const Point& point
  )
  {
    Point mid1 = midPoint(pPoint, tPoint);
    Point mid2 = midPoint(point, pPoint);
    
    PathFigureCollection pathFigureCollection = path.Data().as<PathGeometry>().Figures();
    PathFigure pathFigure;
    pathFigure.StartPoint(mid1);
    pathFigureCollection.Append(pathFigure);
    PathSegmentCollection segmentCollection;
    QuadraticBezierSegment segment;
    segment.Point1(pPoint);
    segment.Point2(mid2);
    segmentCollection.Append(segment);
    pathFigure.Segments(segmentCollection);

  }

}
