import unittest
from numpy import array
from numpy import array_equal

from navigation.lane_piece import LanePiece


class LanePieceTest(unittest.TestCase):
    def test_WhenConstructed_ThenCheckProperties(self):
        lane_piece = LanePiece(width=3.2, points=array([[0, 0, 0], [1, 0, 0]]))
        self.assertAlmostEqual(3.2, lane_piece.Width())
        self.assertTrue(
            array_equal(array([[0, 0, 0], [1, 0, 0]]), lane_piece.Points()))

    def test_WhenLaneIsSingle_ThenCheckLength(self):
        lane_piece = LanePiece(width=3.2, points=array(
            [[0, 0, 0], [1, 0, 0]]))
        self.assertAlmostEqual(1, lane_piece.Length())

    def test_WhenLaneIsCurved_ThenCheckLength(self):
        lane_piece = LanePiece(width=3.2, points=array(
            [[0, 0, 0], [1, 0, 0], [1, 1, 0]]))
        self.assertAlmostEqual(2, lane_piece.Length())

    def test_WhenLaneIsLine_ThenCheckLength(self):
        lane_piece = LanePiece(width=3.2, points=array(
            [[0, 0, 0], [1, 0, 0]]))
        self.assertAlmostEqual(1, lane_piece.Length())

    def test_WhenLocationIsInLine_ThenCheckLength(self):
        lane_piece = LanePiece(width=3.2, points=array(
            [[0, 0, 0], [1, 0, 0]]))
        location = array([0.2, 1.0, 0])
        self.assertAlmostEqual(
            0.2, lane_piece.LengthFromStartToLocation(location))
        self.assertAlmostEqual(
            0.8, lane_piece.LengthFromLocationToEnd(location))

    def test_WhenLocationIsBeforeLine_ThenCheckLength(self):
        lane_piece = LanePiece(width=3.2, points=array(
            [[0, 0, 0], [1, 0, 0]]))
        location = array([-0.2, 1.0, 0])
        self.assertAlmostEqual(
            0.0, lane_piece.LengthFromStartToLocation(location))
        self.assertAlmostEqual(
            1.0, lane_piece.LengthFromLocationToEnd(location))

    def test_WhenLocationIsAfterLine_ThenCheckLength(self):
        lane_piece = LanePiece(width=3.2, points=array(
            [[0, 0, 0], [1, 0, 0]]))
        location = array([1.5, 1.0, 0])
        self.assertAlmostEqual(
            1.0, lane_piece.LengthFromStartToLocation(location))
        self.assertAlmostEqual(
            0.0, lane_piece.LengthFromLocationToEnd(location))

    def test_WhenLocationIsInLine_ThenCheckDistance(self):
        lane_piece = LanePiece(width=3.2, points=array(
            [[0, 0, 0], [1, 0, 0]]))
        location = array([0.2, 0.5, 0])
        self.assertAlmostEqual(
            0.5, lane_piece.DistanceToLocation(location))

    def test_WhenLocationIsBeforeLine_ThenCheckDistance(self):
        lane_piece = LanePiece(width=3.2, points=array(
            [[0, 0, 0], [1, 0, 0]]))
        location = array([-3.0, 4.0, 0])
        self.assertAlmostEqual(
            5.0, lane_piece.DistanceToLocation(location))

    def test_WhenLocationIsAfterLine_ThenCheckDistance(self):
        lane_piece = LanePiece(width=3.2, points=array(
            [[0, 0, 0], [1, 0, 0]]))
        location = array([4.0, 4.0, 0])
        self.assertAlmostEqual(
            5.0, lane_piece.DistanceToLocation(location))


if __name__ == '__main__':
    unittest.main()
