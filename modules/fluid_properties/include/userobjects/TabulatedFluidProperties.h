/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef TABULATEDFLUIDPROPERTIES_H
#define TABULATEDFLUIDPROPERTIES_H

#include "SinglePhaseFluidPropertiesPT.h"

class SinglePhaseFluidPropertiesPT;
class BicubicSplineInterpolation;
class TabulatedFluidProperties;

template <>
InputParameters validParams<TabulatedFluidProperties>();

/**
 * Class for fluid properties read from a file.
 *
 * Property values are read from a file containing keywords followed by data.
 * Monotonically increasing values of pressure and temperature must be included in
 * the data file, specifying the phase space where tabulated fluid properties will
 * be defined. An error is thrown if either temperature or pressure data is not
 * included or not monotonic, and an error is also thrown if this UserObject is
 * requested to provide a fluid property outside this phase space.
 *
 * This class is intended to be used when complicated formulations for density,
 * internal energy or enthalpy are required, which can be computationally expensive.
 * This is particularly the case where the fluid equation of state is based on a
 * Helmholtz free energy that is a function of density and temperature, like that
 * used in CO2FluidProperties. In this case, density must be solved iteratively using
 * pressure and temperature, which increases the computational burden.
 *
 * In these cases, using an interpolation of the tabulated fluid properties can
 * significantly reduce the computational time for computing density, internal energy,
 * and enthalpy.
 *
 * The expected file format for the tabulated fluid properties is now described.
 * Lines beginning with # are ignored, so comments can be included.
 * Keywords 'pressure' and 'temperature' must be included, each followed by numerical data
 * that increases monotonically. A blank line signifies the end of the data for the
 * preceding keyword.
 *
 * Fluid properties for density, internal energy, and enthalpy can be included, with
 * the keyword 'density', 'internal_energy', or 'enthalpy' followed by data that cycles
 * first by temperature then pressure. If any of these properties are not supplied,
 * this UserObject will generate it using the pressure and temperature values provided.
 * An error is thrown if an incorrect number of property values has been supplied.
 *
 * If no tabulated fluid property data file exists, then data for density, internal energy
 * and enthalpy will be generated using the pressure and temperature ranges specified
 * in the input file at the beginning of the simulation.
 *
 * This tabulated data will be written to file in the correct format,
 * enabling suitable data files to be created for future use. There is an upfront
 * computational expense required for this initial data generation, depending on the
 * required number of pressure and temperature points. However, provided that the
 * number of data points required to generate the tabulated data is smaller than the
 * number of times the property members in the FluidProperties UserObject are used,
 * the initial time to generate the data and the subsequent interpolation time can be much
 * less than using the original FluidProperties UserObject.
 *
 * Density, internal_energy and enthalpy and there derivatives wrt pressure and
 * temperature are always calculated using bicubic spline interpolatio, while all
 * remaining fluid properties are calculated using the FluidProperties UserObject _fp.
 *
 * A function to write generated data to file using the correct format is provided
 * to allow suitable files of fluid property data to be generated using the FluidProperties
 * module UserObjects.
 */
class TabulatedFluidProperties : public SinglePhaseFluidPropertiesPT
{
public:
  TabulatedFluidProperties(const InputParameters & parameters);
  virtual ~TabulatedFluidProperties();

  virtual void initialSetup() override;

  /// Fluid name
  virtual std::string fluidName() const override;
  /// Molar mass
  virtual Real molarMass() const override;
  /// Density
  virtual Real rho(Real pressure, Real temperature) const override;
  /// Density and its derivatives wrt pressure and temperature
  virtual void rho_dpT(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;
  /// Internal energy
  virtual Real e(Real pressure, Real temperature) const override;
  /// Internal energy and its derivatives wrt pressure and temperature
  virtual void
  e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const override;
  /// Density and internal energy, and derivatives wrt pressure and temperature
  virtual void rho_e_dpT(Real pressure,
                         Real temperature,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_dT,
                         Real & e,
                         Real & de_dp,
                         Real & de_dT) const override;
  /// Enthalpy
  virtual Real h(Real p, Real T) const override;
  /// Enthalpy and its derivatives wrt pressure and temperature
  virtual void
  h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;
  /// Viscosity
  virtual Real mu(Real density, Real temperature) const override;
  /// Derivatives of viscosity wrt density and temperature
  virtual void mu_drhoT(
      Real density, Real temperature, Real & mu, Real & dmu_drho, Real & dmu_dT) const override;
  /// Specific isobaric heat capacity
  virtual Real cp(Real pressure, Real temperature) const override;
  /// Specific isochoric heat capacity
  virtual Real cv(Real pressure, Real temperature) const override;
  /// Speed of sound
  virtual Real c(Real pressure, Real temperature) const override;
  /// Thermal conductivity
  virtual Real k(Real density, Real temperature) const override;
  /// Specific entropy
  virtual Real s(Real pressure, Real temperature) const override;
  /// Thermal expansion coefficient
  virtual Real beta(Real pressure, Real temperature) const override;
  /// Henry's law constant for dissolution in water
  virtual Real henryConstant(Real temperature) const override;
  /// Henry's law constant for dissolution in water and derivative wrt temperature
  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const override;

protected:
  /**
   * Read tabulated data from a file and store it in vectors
   * @param file_name name of the file to be read
   */
  void parseTabulatedData(std::string file_name);

  /**
   * Helper function to parse lines of data read from file.
   * @param line line to be parsed
   * @param[out] data vector of data parsed from line
   */
  void parseData(std::string line, std::vector<Real> & data);

  /**
   * Writes tabulated data to a file.
   * @param file_name name of the file to be written
   */
  void writeTabulatedData(std::string file_name);

  /**
   * Checks that the inputs are within the range of the tabulated data, and throws
   * an error if they are not.
   * @param pressure input pressure (Pa)
   * @param temperature input temperature (K)
   */
  void checkInputVariables(Real pressure, Real temperature) const;

  /**
   * Generates a table of fluid properties by looping over pressure and temperature
   * and calculating properties using the FluidProperties UserObject _fp.
   */
  virtual void generateAllTabulatedData();

  /**
   * Generates any missing data that has been parsed from an input file. For example,
   * if one of the required properties has not been included, then it is generated using
   * the FluidProperties UserObject at the pressure and temperature points given in the
   * data file.
   */
  virtual void generateMissingTabulatedData();

  /**
   * Forms a 2D matrix from a single std::vector.
   * @param nrow number of rows in the matrix
   * @param ncol number of columns in the matrix
   * @param vec 1D vector to reshape into a 2D matrix
   * @param[out] 2D matrix formed by reshaping vec
   */
  void reshapeData2D(unsigned int nrow,
                     unsigned int ncol,
                     std::vector<Real> & vec,
                     std::vector<std::vector<Real>> & mat);

  /**
   * Forms a 1D vector from a 2D matrix.
   * @param mat 2D matrix
   * @return 1D vector containing elements of mat
   */
  std::vector<Real> flattenData(std::vector<std::vector<Real>> & mat);

  /// File name of tabulated data file
  FileName _file_name;
  /// Pressure vector
  std::vector<Real> _pressure;
  /// Temperature vector
  std::vector<Real> _temperature;
  /// Tabulated density
  std::vector<std::vector<Real>> _density;
  /// Tabulated internal energy
  std::vector<std::vector<Real>> _internal_energy;
  /// Tabulated enthalpy
  std::vector<std::vector<Real>> _enthalpy;
  /// Interpoled density
  std::unique_ptr<BicubicSplineInterpolation> _density_ipol;
  /// Interpoled internal energy
  std::unique_ptr<BicubicSplineInterpolation> _internal_energy_ipol;
  /// Interpoled enthalpy
  std::unique_ptr<BicubicSplineInterpolation> _enthalpy_ipol;
  /// Derivatives along the boundary
  std::vector<Real> _drho_dp_0, _drho_dp_n, _drho_dT_0, _drho_dT_n;
  std::vector<Real> _de_dp_0, _de_dp_n, _de_dT_0, _de_dT_n;
  std::vector<Real> _dh_dp_0, _dh_dp_n, _dh_dT_0, _dh_dT_n;

  /// Minimum temperature in tabulated data
  Real _temperature_min;
  /// Maximum temperature in tabulated data
  Real _temperature_max;
  /// Minimum pressure in tabulated data
  Real _pressure_min;
  /// Maximum pressure in tabulated data
  Real _pressure_max;
  /// Number of temperature points in the tabulated data
  unsigned int _num_T;
  /// Number of pressure points in the tabulated data
  unsigned int _num_p;
  /// Index for derivatives wrt pressure
  const unsigned int _wrt_p = 1;
  /// Index for derivatives wrt temperature
  const unsigned int _wrt_T = 2;

  /// SinglePhaseFluidPropertiesPT UserObject
  const SinglePhaseFluidPropertiesPT & _fp;

  /// List of reuired axes names to be read
  const std::vector<std::string> _required_axes{"pressure", "temperature"};
  /// List of valid fluid property names that can be read
  const std::vector<std::string> _valid_props{"density", "enthalpy", "internal_energy"};
};

#endif /* TABULATEDFLUIDPROPERTIES_H */
