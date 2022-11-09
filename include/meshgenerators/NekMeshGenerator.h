/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#pragma once

#include "MeshGenerator.h"

/**
 * Base class for converting a MOOSE mesh into a NekRS-compatible format,
 * with options to preserve circular edges.
 */
class NekMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  NekMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  /**
   * Extract all user specifications for the circular sideset movements
   */
  void getCircularSidesetInfo(std::unique_ptr<MeshBase> & mesh);

  /**
   * Extract all user specifications for the polygon corner smoothing
   */
  std::vector<Real> getPolygonSmoothingInfo(std::unique_ptr<MeshBase> & mesh);

  /**
   * Extract all user specifications for the boundaries to rebuild
   */
  void getBoundariesToRebuild(std::unique_ptr<MeshBase> & mesh);

  /**
   * Check for correct element type in input mesh
   */
  virtual void checkElementType(std::unique_ptr<MeshBase> & mesh) = 0;

  /**
   * Get the two nodes to average coordinates for given a side-node id
   * @param[in] node_id mid-point side node ID
   * @return two nodes on the corners to average to get new coordinates
   */
  virtual std::pair<unsigned int, unsigned int> pairedNodesAboutMidPoint(const unsigned int & node_id) const = 0;

  /**
   * Get a node on the given face that is unique to that side of the element
   * @param[in] primary_face face ID
   * @return node index on that face
   */
  unsigned int getFaceNode(const unsigned int & primary_face) const;

  /**
   * Get the node indices on a given face
   * @param[in] face face index
   * @return node indices on that face
   */
  virtual const std::vector<unsigned int> nodesOnFace(const unsigned int & face) const = 0;

  /**
   * Whether a node on a face is a corner node
   * @param[in] node node index
   * @return whether node is a corner node
   */
  virtual bool isCornerNode(const unsigned int & node) const = 0;

  /**
   * Store the previous mesh information in order to rebuild it later
   */
  void storeMeshInfo(std::unique_ptr<MeshBase> & mesh, std::vector<dof_id_type> & boundary_elem_ids,
    std::vector<unsigned int> & boundary_face_ids, std::vector<std::vector<boundary_id_type>> & boundary_ids);

  /**
   * Create the sidesets in the new mesh
   */
  void addSidesets(std::unique_ptr<MeshBase> & mesh, std::vector<dof_id_type> & boundary_elem_ids,
    std::vector<unsigned int> & boundary_face_ids,
    std::vector<std::vector<boundary_id_type>> & boundary_ids);

  /**
   * Move any elements for circular adjusting
   */
  void moveNodes(std::unique_ptr<MeshBase> & mesh, std::vector<Real> & polygon_layer_smoothing);

  /**
   * Save the previous node positions and rebuild them with the new element order
   */
  void saveAndRebuildNodes(std::unique_ptr<MeshBase> & mesh);

  /**
   * Get a pointer to the next element in the boundary layer
   * @param[in] elem current element in the boundary layer
   * @param[in] primary_face face of the element that touches the "next" element
   * @param[out] next_touching_face face ID of the "next" element
   */
  const Elem * getNextLayerElem(const Elem & elem, const unsigned int & touching_face,
    unsigned int & next_touching_face) const;

  /**
   * Get the node index corresponding to the mid-point side for a given corner node and face
   * @param[in] face_id face ID of the element
   * @param[in] face_node node on given face
   * @return index of the mid-point node for that corner node "perpendicular" from the face
   */
  unsigned int midPointNodeIndex(const unsigned int & face_id, const unsigned int & face_node) const;

  /**
   * Get the node "paired" to the present node "across" the face
   * @param[in] node_id node ID on primary face
   * @param[in] face_id ID of primary face
   * @return node ID "across" that face to the other side of the Hex27
   */
  unsigned int pairedFaceNode(const unsigned int & node_id, const unsigned int & face_id) const;

  /**
   * Given an element with a face on the boundary of interest, get pointers
   * to all the elements nested into the geometry
   * @param[in] elem input element
   * @param[in] n_layers number of layers to sweep inwards
   * @param[in] primary_face face of the input element that is on the boundary
   * @return vector of elements attached to input element
   */
  std::vector<Elem *> getBoundaryLayerElems(Elem * elem, const unsigned int & n_layers,
    const unsigned int & primary_face) const;

  /**
   * For an element on the 'boundary' of interest, move its nodes, as well as those
   * of any boundary layers
   * @param[in] elem element on the primary 'boundary'
   * @param[in] boundary_index index of the 'boundary'
   * @param[in] primary_face face ID of the element on the 'boundary'
   */
  void moveElem(Elem * elem, const unsigned int & boundary_index, const unsigned int & primary_face,
    const std::vector<Real> & polygon_layer_smoothing);

  /**
   * Whether a point is close enough to a corner to require moving to fit the
   * curved radii of curvature
   * @param[in] pt point
   * @return whether point needs to be moved
   */
  bool isNearCorner(const Point & pt) const;

  /**
   * Get the node index pertaining to a given point
   * @param[in] elem element
   * @param[in] pt point of interest
   * @return node index corresponding to the point
   */
  unsigned int getNodeIndex(const Elem * elem, const Point & pt) const;

  /**
   * Find the origin closest to the given point
   * @param[in] index boundary index to look for origins
   * @param[in] pt point of interest
   * @return closest origin
   */
  Point getClosestOrigin(const unsigned int & index, const Point & pt) const;

  /**
   * Check that points have 3 components to represent x, y, z
   * @param[in] points points
   * @param[in] name name of input parameter for error messages
   */
  void checkPointLength(const std::vector<std::vector<Real>> & points, std::string name) const;

  /**
   * Adjust the mid-point side node to the average of the corners
   * @param[in] node_id index for the node on the side mid-point
   * @param[in] elem element of interest
   * @return average of the two corner nodes corresponding to the node_id mid-side node
   */
  void adjustMidPointNode(const unsigned int & node_id, Elem * elem) const;

  /**
   * Get the boundary ID pertaining to a boundary name, and check for validity
   * @param[in] name boundary to check
   * @param[in] mesh mesh
   * @return boundary ID corresponding to name
   */
  BoundaryID getBoundaryID(const BoundaryName & name, const MeshBase & mesh) const;

  /**
   * Project a point onto the (x,y) plane (perpendicular to the 'axis'), relative to the origin
   * @param[in] origin origin
   * @param[in] pt point to project
   * @return projected point
   */
  Point projectPoint(const Point & origin, const Point & pt) const;

  /**
   * Adjust the point to which a face node should move to fit onto the circle
   * @param[in] node_id node ID
   * @param[in] elem element of interest
   * @return adjustment made to point, for use when shifting other boundary layers
   */
  Point adjustPointToCircle(const unsigned int & node_id, Elem * elem, const Real & radius, const Point & origin) const;

protected:
  /// Mesh to modify
  std::unique_ptr<MeshBase> & _input;

  /// Axis of the mesh about which to build the circular surface
  const MooseEnum & _axis;

  /// Whether to move corner nodes to fit curved radii of a regular polygon
  const bool & _curve_corners;

  /**
   * When curving corners, this mesh generator assumes that the polygon boundary
   * is oriented so that it has a flat side "horizontal." For instance, if the
   * polygon has 3 sides, this would look like:
   *
   *     o ----- o
   *      \     /
   *       \   /
   *        \o/
   *
   * This is only used for identifying the points "at the corners" that need
   * to be modified. If the input mesh therefore does not match the above, this
   * rotation angle can be used to specify a different polygon orientation for
   * the sake of curving corners.
   */
  const Real & _rotation_angle;

  /// Whether sidesets will be moved to match circular surfaces
  const bool _has_moving_boundary;

  /// Number of nodes per element in original mesh
  unsigned int _n_start_nodes;

  /// Number of nodes per element in resulting mesh
  unsigned int _n_end_nodes;

  /// Number of nodes per side of the starting element
  unsigned int _n_start_nodes_per_side;

  /// Number of sides per element (faces in 3-D, edges in 2-D)
  unsigned int _n_sides;

  /// Sideset IDs to move to fit the circular surface
  std::vector<BoundaryID> _moving_boundary;

  /// Radii of the circular surface(s)
  std::vector<Real> _radius;

  /// Origins of the circular surface(s)
  std::vector<std::vector<Real>> _origin;

  /// Number of layers to sweep on each boundary
  std::vector<unsigned int> _layers;

  /// If curving corners, the radius of curvature of the corner
  Real _corner_radius;

  /// If curving corners, the corner coordinates of the polygon
  std::vector<Point> _polygon_corners;

  /// Maximum distance a point can be from a corner and still require movement to the curved corners
  Real _max_corner_distance;

  /// Number of boundaries to be moved that do not include the corners
  unsigned int _n_noncorner_boundaries;

  /// Boundaries to rebuild in the new mesh
  std::set<BoundaryID> _boundaries_to_rebuild;

  /// Original boundary names and IDs
  std::map<BoundaryID, BoundaryName> _boundary_id_to_name;

  /// For each face, the mid-side nodes to be adjusted
  std::vector<std::vector<unsigned int>> _side_ids;

  /// For each face, the corner node indices
  std::vector<std::vector<unsigned int>> _corner_nodes;

  /// For each face, the node pairing "across" that face to the other side of the original elem type
  std::vector<std::vector<std::pair<unsigned int, unsigned int>>> _across_pair;

  /// For each face, the paired face "across" to the other side of the original elem type
  std::vector<unsigned int> _across_face;
};
