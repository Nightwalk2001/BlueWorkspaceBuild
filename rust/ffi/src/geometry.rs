use serde::Serialize;
use smartstring::alias::String;

#[derive(Clone, Copy, Serialize)]
pub(crate) struct Point {
    x: f32,
    y: f32,
}

impl From<&layout::Point> for Point {
    fn from(value: &layout::Point) -> Self {
        Self {x: value.x, y: value.y}
    }
}

/// Rust Impl of [`d3-shape`] curveBasis algorithm
pub(crate) fn line_curve(points: &[Point]) -> String {
    let n = points.len();
    if n < 3 {
        return String::new();
    }

    let mut path = format!("M{},{}", points[0].x, points[0].y);

    for i in 1..n - 2 {
        let p1 = points[i];
        let p2 = points[i + 1];
        let p3 = points[i + 2];

        let x1 = (p1.x + p2.x) / 2.0;
        let y1 = (p1.y + p2.y) / 2.0;

        let x2 = (p2.x + p1.x) / 2.0;
        let y2 = (p2.y + p1.y) / 2.0;

        let x3 = (2.0 * p2.x + p3.x) / 3.0;
        let y3 = (2.0 * p2.y + p3.y) / 3.0;

        path.push_str(&format!("C{},{},{},{},{},{}", x1, y1, x2, y2, x3, y3));
    }

    let p1 = points[n - 2];
    let p2 = points[n - 1];

    let x1 = (p1.x + p2.x) / 2.0;
    let y1 = (p1.y + p2.y) / 2.0;

    let x2 = p2.x;
    let y2 = p2.y;

    path.push_str(&format!("C{},{},{},{},{},{}", x1, y1, x2, y2, x2, y2));

    String::from(path)
}

type BoundsArray = (i32, i32, i32, i32);

/// ### Safety
/// layout phase guarantee points has at least two points
pub(crate) fn calc_edge_bounding(points: &[Point]) -> BoundsArray {
    let mut min_x = f32::INFINITY;
    let mut min_y = f32::INFINITY;
    let mut max_x = f32::NEG_INFINITY;
    let mut max_y = f32::NEG_INFINITY;

    for point in points {
        min_x = min_x.min(point.x);
        min_y = min_y.min(point.y);
        max_x = max_x.max(point.x);
        max_y = max_y.max(point.y);
    }

    (
        min_x.round() as i32,
        min_y.round() as i32,
        (max_x - min_x).round() as i32,
        (max_y - min_y).round() as i32,
    )
}
