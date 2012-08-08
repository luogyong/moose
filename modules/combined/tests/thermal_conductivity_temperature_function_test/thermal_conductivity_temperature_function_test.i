#
# This test evaluates the capability of HeatConductionMaterial to define
# thermal conductivity as a function of temperature.  The test uses the patch test
# cube mesh with a flux bc on one side and a temperature bc on the opposite side.
# The temperature bc changes as a function of time from 100 to 200.  The thermal
# conductivity is a function of temperature, with k = 1 for temps = 100-199, k = 2
# for temps _>_ 200. The flux, q = 10 is constant.  The Transient Executioner is used here
# although the interial kernel is omitted, so this is really a series of two steady-state
# solutions.
#
#                         ---------------
#                        |               |
#                        |               |
#                q    -> |       k       |  T2
#                        |               |
#             T1 = ?     |               |
#                         ---------------
#                              dx = 1
#
#
#                         q = -k dT/dx
#
#                         q = -k (T1 - T2)/dx
#
#                         T1 = (q/-k)*dx + T2
#
#                         for: T2 = 100, k = 1, q = -10
#                                       
#                         T1 = 110 
#                         --------
#
#                         for: T2 = 200, k = 2, q = -10
#                                       
#                         T1 = 205
#                         -------- 
#

[Mesh]#Comment
  file = fe_patch.e
[] # Mesh

[Functions]
  [./k_func]
    type = PiecewiseLinear
    x = '100 199 200'
    y = '1   1   2'
  [../]

  [./t_func]
    type = PiecewiseLinear
    x = '0   1   2'
    y = '100 100 200'
  [../]
[] # Functions

[Variables]

  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 100
  [../]

[] # Variables

[Kernels]

  [./heat_r]
    type = HeatConduction
    variable = temp
  [../]


[] # Kernels

[BCs]

  [./temps_function]
    type = FunctionPresetBC
    variable = temp
    boundary = 1000
    function = t_func
  [../]

  [./flux_in]
    type = NeumannBC
    variable = temp
    boundary = 100
    value = 10
  [../]

[] # BCs

[Materials]

  [./heat]
    type = HeatConductionMaterial
    block = 1
    specific_heat = 0.116
    thermal_conductivity = 1
    temp = temp
    thermal_conductivity_temperature_function = k_func
  [../]

  [./density]
    type = Density
    block = 1
    density = 0.283
  [../]

[] # Materials


[Executioner]

  type = Transient

  petsc_options = '-snes_mf_operator -ksp_monitor'
  petsc_options_iname = '-pc_type -snes_type -snes_ls -ksp_gmres_restart'
  petsc_options_value = 'lu       ls         basic    101'

  l_max_its = 100
  l_tol = 8e-3

  nl_max_its = 15
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 1
  end_time = 2
  num_steps = 2


[] # Executioner

[Output]
  file_base = out
  interval = 1
  output_initial = true
  exodus = true
  perf_log = true
[] # Output
