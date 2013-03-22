#ifndef IMAGEWRAPPERBASE_H
#define IMAGEWRAPPERBASE_H

#include "SNAPCommon.h"
#include "ImageCoordinateTransform.h"
#include "itkImageRegion.h"
#include "itkObject.h"
#include "SNAPEvents.h"

namespace itk {
  template <unsigned int VDim> class ImageBase;
  template <class TPixel, unsigned int VDim> class Image;
  template <class TPixel> class RGBAPixel;

  namespace Statistics {
    class DenseFrequencyContainer;
    template <class TReal, unsigned int VDim, class TContainer> class Histogram;
  }
}

class ScalarImageWrapperBase;
class VectorImageWrapperBase;
class IntensityCurveInterface;
class ScalarImageHistogram;
class ColorMap;
class ImageCoordinateGeometry;
class AbstractNativeIntensityMapping;
class AbstractDisplayMappingPolicy;
class SNAPSegmentationROISettings;

/**
 \class ImageWrapper
 \brief Abstract parent class for all image wrappers

 This class is at the head of the ImageWrapper hierarchy. In fact, there are
 two parallel hierarchies: the untyped hierarchy (xxxWrapperBase) and the
 hierarchy templated over a type (xxxWrapper).

 The idea is that most SNAP code will work with the untyped hierarches. Thus,
 the code will not know what the underlying format of the image is. The typed
 hierarchy is invisible to most of the SNAP classes, and accessed on special
 occasions, where the raw data of the image is needed.
 */
class ImageWrapperBase : public itk::Object
{
public:

  // Definition for the display slice type
  typedef itk::RGBAPixel<unsigned char> DisplayPixelType;
  typedef itk::Image<DisplayPixelType,2> DisplaySliceType;
  typedef SmartPtr<DisplaySliceType> DisplaySlicePointer;

  // Image base
  typedef itk::ImageBase<3> ImageBaseType;

  // Transform matrices
  typedef vnl_matrix_fixed<double, 4, 4> TransformType;

  // Image wrappers can fire certain events
  FIRES(WrapperMetadataChangeEvent)

  virtual ~ImageWrapperBase() { }

  /**
    Get a unique id for this wrapper. All wrappers ever created have
    different ids.
    */
  virtual unsigned long GetUniqueId() const = 0;

  /**
   * Every wrapper, whether it is a scalar wrapper or a vector wrapper, has a
   * scalar representation. For scalar wrappers, this function just returns a
   * pointer to itself. For vector wrappers, the behavior of this function
   * depends on which scalar representation has been selected as the default
   * scalar representation (e.g., one of the components, magnitude, max, mean).
   */
  virtual ScalarImageWrapperBase *GetDefaultScalarRepresentation() = 0;

  /** Get the coordinate transform for each display slice */
  virtual const ImageCoordinateTransform &GetImageToDisplayTransform(
    unsigned int) const = 0;

  /**
   * Set the trasforms from image space to one of the three display slices (be
   * sure to set all three, or you'll get weird looking slices!
   */
  virtual void SetImageToDisplayTransform(
    unsigned int, const ImageCoordinateTransform &) = 0;

  /**
   * Use a default image-slice transformation, the first slice is along z,
   * the second along y, the third, along x, all directions of traversal are
   * positive.
   */
  virtual void SetImageToDisplayTransformsToDefault() = 0;

  /**
   * Update the image coordinate geometry of the image wrapper. This method
   * sets the image's direction cosine matrix and updates the slicers. It is
   * used when the orientation of the image is changed
   */
  virtual void SetImageGeometry(const ImageCoordinateGeometry &geom) = 0;

  /** Get the current slice index */
  irisVirtualGetMacro(SliceIndex, Vector3ui)

  /**
   * Set the current slice index in all three dimensions.  The index should
   * be specified in the image coordinates, the slices will be generated
   * in accordance with the transforms that are specified
   */
  virtual void SetSliceIndex(const Vector3ui &) = 0;

  /** Return some image info independently of pixel type */
  irisVirtualGetMacro(ImageBase, ImageBaseType *)

  /**
   * Is the image initialized?
   */
  irisVirtualIsMacro(Initialized)

  /** Is this image of scalar type? */
  virtual bool IsScalar() const = 0;

  /**
   * Get the size of the image
   */
  virtual Vector3ui GetSize() const = 0;

  /** Get layer transparency */
  irisVirtualSetMacro(Alpha, unsigned char)

  /** Set layer transparency */
  irisVirtualGetMacro(Alpha, unsigned char)

  /** Switch on/off visibility */
  virtual void ToggleVisibility() = 0;

  /**
   * Get the buffered region of the image
   */
  virtual itk::ImageRegion<3> GetBufferedRegion() const = 0;

  /** Transform a voxel index into a spatial position */
  virtual Vector3d TransformVoxelIndexToPosition(const Vector3ui &iVoxel) const = 0;

  /** Transform a voxel index into NIFTI coordinates (RAS) */
  virtual Vector3d TransformVoxelIndexToNIFTICoordinates(const Vector3d &iVoxel) const = 0;

  /** Transform NIFTI coordinates to a continuous voxel index */
  virtual Vector3d TransformNIFTICoordinatesToVoxelIndex(const Vector3d &vNifti) const = 0;

  /** Get the NIFTI s-form matrix for this image */
  irisVirtualGetMacro(NiftiSform, TransformType)

  /** Get a display slice correpsponding to the current index */
  virtual DisplaySlicePointer GetDisplaySlice(unsigned int dim) = 0;

  /** For each slicer, find out which image dimension does is slice along */
  virtual unsigned int GetDisplaySliceImageAxis(unsigned int slice) = 0;

  /** Get the number of voxels */
  virtual size_t GetNumberOfVoxels() const = 0;

  /** Get the number of components per voxel */
  virtual size_t GetNumberOfComponents() const = 0;

  /** Get voxel at index as an array of double components */
  virtual void GetVoxelAsDouble(const Vector3ui &x, double *out) const = 0;

  /** Get voxel at index as an array of double components */
  virtual void GetVoxelAsDouble(const itk::Index<3> &idx, double *out) const = 0;

  /** Get voxel intensity in native space. These methods are not recommended
      for iterating over the entire image, since there is a virutal method
      being resolved at each iteration. */
  virtual void GetVoxelMappedToNative(const Vector3ui &vec, double *out) const = 0;
  virtual void GetVoxelMappedToNative(const itk::Index<3> &idx, double *out) const = 0;

  /** Return componentwise minimum cast to double, without mapping to native range */
  virtual double GetImageMinAsDouble() = 0;

  /** Return componentwise maximum cast to double, without mapping to native range */
  virtual double GetImageMaxAsDouble() = 0;

  /** Return componentwise minimum cast to double, after mapping to native range */
  virtual double GetImageMinNative() = 0;

  /** Return componentwise maximum cast to double, after mapping to native range */
  virtual double GetImageMaxNative() = 0;

  /**
    Get the RGBA apperance of the voxel at the intersection of the three
    display slices.
    */
  virtual void GetVoxelUnderCursorAppearance(DisplayPixelType &out) = 0;

  /**
   * This method returns a vector of values for the voxel under the cursor.
   * This is the natural value or set of values that should be displayed to
   * the user. The value depends on the current display mode. For scalar
   * images, it's just the value of the voxel, but for multi-component images,
   * it's the value of the selected component (if there is one) or the value
   * of the multiple components when the mode is RGB.
   */
  virtual vnl_vector<double> GetVoxelUnderCursorDisplayedValue() = 0;

  /** Get the voxel array, as void pointer */
  virtual void *GetVoxelVoidPointer() const = 0;

  /** Clear the data associated with storing an image */
  virtual void Reset() = 0;

  /**
   * Get the mapping between the internal data type and the 'native' range,
   * i.e., the range of values shown to the user. This may be a linear mapping
   * or an identity mapping.
   */
  virtual const AbstractNativeIntensityMapping *GetNativeIntensityMapping() const = 0;

  /**
   * Get the display mapping policy. This policy differs from wrapper to wrapper
   * and may involve using color labels or color maps.
   */
  virtual AbstractDisplayMappingPolicy *GetDisplayMapping() = 0;

  // Access the filename
  irisVirtualGetStringMacro(FileName)
  irisVirtualSetStringMacro(FileName)

  // Access the nickname
  irisVirtualGetMacro(Nickname, const std::string &)
  irisVirtualSetMacro(Nickname, const std::string &)

  /**
    Export one of the slices as a thumbnail (e.g., PNG file)
    */
  virtual void WriteThumbnail(const char *filename, unsigned int maxdim) = 0;

  /**
   * This static function constructs a NIFTI matrix from the ITK direction
   * cosines matrix and Spacing and Origin vectors
   */
  static TransformType ConstructNiftiSform(
    vnl_matrix<double> m_dir,
    vnl_vector<double> v_origin,
    vnl_vector<double> v_spacing);

  static TransformType ConstructVTKtoNiftiTransform(
    vnl_matrix<double> m_dir,
    vnl_vector<double> v_origin,
    vnl_vector<double> v_spacing);

  typedef itk::Image<short, 3> ShortImageType;

protected:

};

class ScalarImageWrapperBase : public virtual ImageWrapperBase
{
public:

  // A common image format to which the contents of the scalar image wrapper
  // may be cast for downstream processing
  typedef itk::Image<GreyType, 3>                      CommonFormatImageType;

  /**
   * An enum of export channel types. Export channels are used to present the
   * internal image as an itk::Image of a fixed type. For efficient memory
   * management, there are separate channels for downstream filters that
   * operate on the whole image and filters that generate single-slice previews
   * in the orthogonal slicing directions
   */
  enum ExportChannel {
    WHOLE_IMAGE=0, PREVIEW_X, PREVIEW_Y, PREVIEW_Z, CHANNEL_COUNT
  };

  /**
   * Get the scaling factor used to convert between intensities stored
   * in this image and the 'true' image intensities
   */
  virtual double GetImageScaleFactor() = 0;

  /** Get voxel at index as a single double value */
  virtual double GetVoxelAsDouble(const Vector3ui &x) const = 0;

  /** Get voxel at index as a single double value */
  virtual double GetVoxelAsDouble(const itk::Index<3> &idx) const = 0;

  /** Get voxel intensity in native space. These methods are not recommended
      for iterating over the entire image, since there is a virutal method
      being resolved at each iteration. */
  virtual double GetVoxelMappedToNative(const Vector3ui &vec) const = 0;
  virtual double GetVoxelMappedToNative(const itk::Index<3> &idx) const = 0;

  /**
    Get the maximum possible value of the gradient magnitude. This will
    compute the gradient magnitude of the image (without Gaussian smoothing)
    and return the maximum. The value will be cached so repeated calls to
    this are not expensive.
    */
  virtual double GetImageGradientMagnitudeUpperLimit() = 0;

  /**
    Get the maximum possible value of the gradient magnitude in native units
    */
  virtual double GetImageGradientMagnitudeUpperLimitNative() = 0;

  /**
    Compute the histogram of the image and store it in the ITK
    histogram object.
    */
  virtual const ScalarImageHistogram *GetHistogram(size_t nBins) = 0;

  /**
   * Extract a GreyType representation from the image wrapper. Note that
   * internally, the scalar image wrapper can be of many itk types, e.g.,
   * it could be a component of a vector image computed dynamically. In
   * order to use the scalar image in downstream filters, we must have a
   * way to map it to some common datatype. If not, we would have to template
   * the downstream filter on the type of the image in the wrapper, which would
   * lead to an exponential explosion of types.
   *
   * There are actually four representations for each image wrapper, one of
   * which is intended for pipelines that act on entire image volumes and the
   * other three intended for use in preview-capable pipelines, which generate
   * output for just one slice. Since ITK only allocates the requested image
   * region, these four representations should not really use much extra memory.
   *
   * However, it is very important that downstream filters use the itk streaming
   * image filter to break up operations into pieces. Without that, there would
   * be unnecessary large memory allocation.
   */
  virtual CommonFormatImageType* GetCommonFormatImage(
      ExportChannel channel = WHOLE_IMAGE) = 0;

  /**
   * Get the intensity curve used to map raw intensities to color map inputs.
   * The intensity curve is only used by some wrappers (anatomic, speed) and
   * so this method may return NULL for some layers.
   */
  virtual IntensityCurveInterface *GetIntensityCurve() const = 0;

  /**
   * Get the color map used to present image intensities as RGBA.
   */
  virtual ColorMap *GetColorMap() const = 0;
};

/**
  This type of image wrapper is meant to represent a continuous range of values
  as opposed to a discrete set of labels. The wrapper contains a color map
  which is used to map from intensity ranges to display pixels
  */
class ContinuousScalarImageWrapperBase : public virtual ScalarImageWrapperBase
{
public:

  /** Set the reference to the color map object */
  virtual ColorMap* GetColorMap() const = 0;


};


class VectorImageWrapperBase : public virtual ImageWrapperBase
{
public:

  /**
   * Supported ways of extracting a scalar value from vector-valued data.
   * These modes allow the image to be cast to a scalar image and used in
   * single-modality pipelines
   */
  enum ScalarRepresentation
  {
    SCALAR_REP_COMPONENT = 0,
    SCALAR_REP_MAGNITUDE,
    SCALAR_REP_MAX,
    SCALAR_REP_AVERAGE,
    NUMBER_OF_SCALAR_REPS
  };


  virtual ScalarImageWrapperBase *GetScalarRepresentation(
      ScalarRepresentation type, int index = 0) = 0;
};

class RGBImageWrapperBase : public virtual VectorImageWrapperBase
{
public:

};

#endif // IMAGEWRAPPERBASE_H
