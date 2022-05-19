// ITK includes
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkMaskImageFilter.h>
#include <itkMaskNegatedImageFilter.h>
#include <itkAbsoluteValueDifferenceImageFilter.h>
#include <itkStatisticsImageFilter.h>

// TCLAP includes
#include <tclap/ValueArg.h>
#include <tclap/ArgException.h>
#include <tclap/CmdLine.h>
#include <tclap/SwitchArg.h>

// STD includes
#include <cstdlib>

// NOTE: For now we will assume images to compare are float and mask is unsigned short

int main (int argc, char **argv)
{

  // =========================================================================
  // Command-line variables
  // =========================================================================
  std::string imageAFileName;
  std::string imageBFileName;
  std::string maskImageFileName;
  std::string maskedImageAFileName;
  std::string maskedImageBFileName;
  std::string differenceImage;
  bool maskOutside = false;
  unsigned short int maskLabel = 0;
  float maskValue = 0;
  float maxTolerance = 0;
  float minTolerance = 0;
  float meanTolerance = 0;
  float sigmaTolerance = 0;

  // =========================================================================
  // Parse arguments
  // =========================================================================
  try {

    TCLAP::CmdLine cmd("itkImageCompare");

    TCLAP::ValueArg<std::string> imageAInput("a", "imageA", "Input Image A", true, "None", "string");
    TCLAP::ValueArg<std::string> imageBInput("b", "imageB", "Input Image B", true, "None", "string");
    TCLAP::ValueArg<std::string> maskImageInput("k", "mask", "MaskImage", false, "None", "string");
    TCLAP::ValueArg<std::string> maskedAInput("A", "maskedA", "Output Masked A", false, "None", "string");
    TCLAP::ValueArg<std::string> maskedBInput("B", "maskedB", "Output Masked B", false, "None", "string");
    TCLAP::ValueArg<std::string> differenceImageInput("d", "differenceImage", "Difference of masked (if enabled) images", false, "None", "string");
    TCLAP::ValueArg<unsigned short> maskLabelInput("l", "mask_label", "Value to consider for masking (0 default)", false, 0, "unsigne short");
    TCLAP::ValueArg<float> maskValueInput("u", "mask_value", "Value to replace masked voxels (0 default)", false, 0, "float");
    TCLAP::SwitchArg outsideMaskInput("o", "outside", "Mask operates outside", false);
    TCLAP::ValueArg<float> maxToleranceInput("M", "maxTolerance", "Maximum max value allowed", false, 0, "float");
    TCLAP::ValueArg<float> minToleranceInput("m", "minTolerance", "Maximum min value allowed", false, 0, "float");
    TCLAP::ValueArg<float> sigmaToleranceInput("s", "sigmaTolerance", "Maximum sigma value allowed", false, 0, "float");
    TCLAP::ValueArg<float> meanToleranceInput("e", "meanTolerance", "Maximum mean value allowed", false, 0, "float");

    cmd.add(imageAInput);
    cmd.add(imageBInput);
    cmd.add(maskImageInput);
    cmd.add(outsideMaskInput);
    cmd.add(maskedAInput);
    cmd.add(maskedBInput);
    cmd.add(maskLabelInput);
    cmd.add(maskValueInput);
    cmd.add(differenceImageInput);
    cmd.add(maxToleranceInput);
    cmd.add(minToleranceInput);
    cmd.add(sigmaToleranceInput);
    cmd.add(meanToleranceInput);

    cmd.parse(argc,argv);

    imageAFileName = imageAInput.getValue();
    imageBFileName = imageBInput.getValue();
    maskImageFileName= maskImageInput.getValue();
    maskOutside = outsideMaskInput.getValue();
    maskedImageAFileName = maskedAInput.getValue();
    maskedImageBFileName = maskedBInput.getValue();
    maskLabel = maskLabelInput.getValue();
    maskValue = maskValueInput.getValue();
    differenceImage = differenceImageInput.getValue();
    maxTolerance = maxToleranceInput.getValue();
    minTolerance = minToleranceInput.getValue();
    meanTolerance = meanToleranceInput.getValue();
    sigmaTolerance = sigmaToleranceInput.getValue();

    if (maskImageFileName == "None" && maskOutside)
    {
        std::cerr << "Outside mask switch should be used together with Mask Image" << std::endl;
        return EXIT_FAILURE;
    }

  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
  }

  // =========================================================================
  // ITK definitions
  // =========================================================================
  using ImageType = itk::Image<float, 3>;
  using MaskType = itk::Image<unsigned short,3>;
  using ImageReaderType = itk::ImageFileReader<ImageType>;
  using ImageWriterType = itk::ImageFileWriter<ImageType>;
  using MaskReaderType = itk::ImageFileReader<MaskType>;
  using MaskImageFilter = itk::MaskImageFilter<ImageType, MaskType, ImageType>;
  using MaskNegatedImageFilter = itk::MaskNegatedImageFilter<ImageType, MaskType, ImageType>;
  using AbsoluteValueDifferenceImageFilter = itk::AbsoluteValueDifferenceImageFilter<ImageType, ImageType, ImageType>;
  using StatisticsImageFilter = itk::StatisticsImageFilter<ImageType>;

  // =========================================================================
  // Image loading and checking
  // =========================================================================
  auto imageAReader = ImageReaderType::New();
  imageAReader->SetFileName(imageAFileName);
  imageAReader->Update();

  auto imageBReader = ImageReaderType::New();
  imageBReader->SetFileName(imageBFileName);
  imageBReader->Update();

  auto maskReader = MaskReaderType::New();
  if (maskImageFileName != "None")
  {
    maskReader->SetFileName(maskImageFileName);
    maskReader->Update();
  }

  // Check wether the images and the mask have the same size
  auto imageASize = imageAReader->GetOutput()->GetLargestPossibleRegion().GetSize();
  auto imageBSize = imageBReader->GetOutput()->GetLargestPossibleRegion().GetSize();

  MaskReaderType::SizeType maskSize;
  if (maskImageFileName != "None")
  {
    maskSize = maskReader->GetOutput()->GetLargestPossibleRegion().GetSize();
  }

  if (imageASize != imageBSize) {
    std::cerr << "Image size are different for A and B" << std::endl;
    return EXIT_FAILURE;
  }

  if (maskImageFileName != "None")
  {
    if (imageASize != maskSize || imageBSize != maskSize)
    {
      std::cerr << "Image size are different for A, B and mask" << std::endl;
      return EXIT_FAILURE;
    }
  }

  // =========================================================================
  // Mask the images
  // =========================================================================
  ImageType::Pointer maskedImageAOutput = imageAReader->GetOutput();
  ImageType::Pointer maskedImageBOutput = imageBReader->GetOutput();

  if (maskImageFileName != "None") {

    if (maskOutside)
    {
      auto maskImageFilterA = MaskNegatedImageFilter::New();
      maskImageFilterA->SetInput(imageAReader->GetOutput());
      maskImageFilterA->SetMaskImage(maskReader->GetOutput());
      maskImageFilterA->SetOutsideValue(maskLabel);
      maskImageFilterA->Update();
      maskedImageAOutput = maskImageFilterA->GetOutput();

      auto maskImageFilterB = MaskNegatedImageFilter::New();
      maskImageFilterB->SetInput(imageBReader->GetOutput());
      maskImageFilterB->SetMaskImage(maskReader->GetOutput());
      maskImageFilterB->SetMaskingValue(maskLabel);
      maskImageFilterB->Update();
      maskedImageBOutput = maskImageFilterB->GetOutput();
    }
    else
    {
      auto maskImageFilterA = MaskImageFilter::New();
      maskImageFilterA->SetInput(imageAReader->GetOutput());
      maskImageFilterA->SetMaskImage(maskReader->GetOutput());
      maskImageFilterA->SetMaskingValue(maskLabel);
      maskImageFilterA->Update();
      maskedImageAOutput = maskImageFilterA->GetOutput();

      auto maskImageFilterB = MaskImageFilter::New();
      maskImageFilterB->SetInput(imageBReader->GetOutput());
      maskImageFilterB->SetMaskImage(maskReader->GetOutput());
      maskImageFilterB->SetMaskingValue(maskLabel);
      maskImageFilterB->Update();
      maskedImageBOutput = maskImageFilterB->GetOutput();
    }

    // =========================================================================
    // Write out the masked images (optional)
    // =========================================================================
    if (maskedImageAFileName != "None")
    {
      auto maskImageAWriter = ImageWriterType::New();
      maskImageAWriter->SetInput(maskedImageAOutput);
      maskImageAWriter->SetFileName(maskedImageAFileName);
      maskImageAWriter->Write();
    }

    if (maskedImageBFileName != "None")
    {
      auto maskImageBWriter = ImageWriterType::New();
      maskImageBWriter->SetInput(maskedImageBOutput);
      maskImageBWriter->SetFileName(maskedImageBFileName);
      maskImageBWriter->Write();
    }
  }

  // =========================================================================
  // Compute the difference image
  // =========================================================================
  auto absoluteValueDifferenceImageFilter = AbsoluteValueDifferenceImageFilter::New();
  absoluteValueDifferenceImageFilter->SetInput1(maskedImageAOutput);
  absoluteValueDifferenceImageFilter->SetInput2(maskedImageBOutput);
  absoluteValueDifferenceImageFilter->Update();


  // =========================================================================
  // Write the difference image to disk (optional)
  // =========================================================================
  if (differenceImage != "None")
  {
    auto differenceWriter = ImageWriterType::New();
    differenceWriter->SetFileName(differenceImage.c_str());
    differenceWriter->SetInput(absoluteValueDifferenceImageFilter->GetOutput());
    differenceWriter->Write();
  }

  // =========================================================================
  // Compute the difference image statistics
  // =========================================================================
  auto imageStatisticsFilter = StatisticsImageFilter::New();
  imageStatisticsFilter->SetInput(absoluteValueDifferenceImageFilter->GetOutput());
  imageStatisticsFilter->Update();

  auto mean = imageStatisticsFilter->GetMean();
  auto max = imageStatisticsFilter->GetMaximum();
  auto min = imageStatisticsFilter->GetMinimum();
  auto sigma = imageStatisticsFilter->GetSigma();

  std::cout << "Mean difference:" << mean << std::endl;
  std::cout << "Max. difference:" << max << std::endl;
  std::cout << "Min. difference:" << min<< std::endl;
  std::cout << "Sigma difference:" << sigma << std::endl;

  if (mean > meanTolerance ||
      max > maxTolerance   ||
      min > minTolerance   ||
      sigma > sigmaTolerance )
  {

    std::cerr << "One of more of the measured statistics are higher than tolerance values" <<  std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
