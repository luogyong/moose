# Checking that gravity head is established in the transient situation when 0<=saturation<=1 (note the less-than-or-equal-to).
# 2phase (PP), 2components, vanGenuchten, constant fluid bulk-moduli for each phase, constant viscosity, constant permeability, Corey relative perm
# For better agreement with the analytical solution (ana_pp), just increase nx

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = -1
  xmax = 0
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./ppwater]
    initial_condition = -0.1
  [../]
  [./ppgas]
    initial_condition = 0
  [../]
[]

[AuxVariables]
  [./massfrac_ph0_sp0]
    initial_condition = 1
  [../]
  [./massfrac_ph1_sp0]
    initial_condition = 0
  [../]
[]


[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = ppwater
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = ppwater
    gravity = '-1 0 0'
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = ppgas
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = ppgas
    gravity = '-1 0 0'
  [../]
[]


[Functions]
  [./ana_ppwater]
    type = ParsedFunction
    vars = 'g B p0 rho0'
    vals = '1 2 pp_water_top 1'
    value = '-B*log(exp(-p0/B)+g*rho0*x/B)' # expected pp at base
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater ppgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[Modules]
  [./FluidProperties]
    [./simple_fluid0]
      type = SimpleFluidProperties
      bulk_modulus = 2
      density0 = 1
      viscosity = 1
      thermal_expansion = 0
    [../]
    [./simple_fluid1]
      type = SimpleFluidProperties
      bulk_modulus = 1
      density0 = 0.1
      viscosity = 0.5
      thermal_expansion = 0
    [../]
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
  [../]
  [./temperature_nodal]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./ppss_qp]
    type = PorousFlow2PhasePP_VG
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    m = 0.5
    al = 1
  [../]
  [./ppss]
    type = PorousFlow2PhasePP_VG
    at_nodes = true
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    m = 0.5
    al = 1
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  [../]
  [./simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
    at_nodes = true
  [../]
  [./simple_fluid0_qp]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  [../]
  [./simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
    at_nodes = true
  [../]
  [./simple_fluid1_qp]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./dens_qp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_nodes = false
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.1
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0  0 2 0  0 0 3'
  [../]
  [./relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 1
    phase = 0
  [../]
  [./relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 1
    phase = 1
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
[]

[Postprocessors]
  [./pp_water_top]
    type = PointValue
    variable = ppwater
    point = '0 0 0'
  [../]
  [./pp_water_base]
    type = PointValue
    variable = ppwater
    point = '-1 0 0'
  [../]
  [./pp_water_analytical]
    type = FunctionValuePostprocessor
    function = ana_ppwater
    point = '-1 0 0'
  [../]
  [./mass_ph0]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
  [../]
  [./mass_ph1]
    type = PorousFlowFluidMass
    fluid_component = 1
    execute_on = 'initial timestep_end'
  [../]

[]

[Preconditioning]
  active = andy
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-12 1E-10 10000'
  [../]
  [./check]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-12 1E-10 10000 test'
  [../]
[]


[Executioner]
  type = Transient
  solve_type = Newton
  [./TimeStepper]
    type = FunctionDT
    time_dt = '1E-3 1E-2 1E-1'
    time_t = '1E-3 1E-2 1E-1'
  [../]
  end_time = 1.0
[]

[Outputs]
  execute_on = 'initial timestep_end'
  file_base = grav02c
  [./csv]
    type = CSV
  [../]
  exodus = true
[]
