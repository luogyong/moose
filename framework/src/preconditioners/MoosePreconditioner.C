/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "MoosePreconditioner.h"
#include "FEProblem.h"
#include "PetscSupport.h"
#include "NonlinearSystem.h"

// libMesh includes
#include "libmesh/numeric_vector.h"

template <>
InputParameters
validParams<MoosePreconditioner>()
{
  InputParameters params = validParams<MooseObject>();
  params.addPrivateParam<FEProblemBase *>("_fe_problem_base");

  MooseEnum pc_side("left right symmetric default", "default");
  params.addParam<MooseEnum>("pc_side", pc_side, "Preconditioning side");
  MooseEnum ksp_norm("none preconditioned unpreconditioned natural default", "unpreconditioned");
  params.addParam<MooseEnum>(
      "ksp_norm", ksp_norm, "Sets the norm that is used for convergence testing");
  params.registerBase("MoosePreconditioner");

#ifdef LIBMESH_HAVE_PETSC
  params += Moose::PetscSupport::getPetscValidParams();
#endif // LIBMESH_HAVE_PETSC

  return params;
}

MoosePreconditioner::MoosePreconditioner(const InputParameters & params)
  : MooseObject(params),
    Restartable(params, "Preconditioners"),
    _fe_problem(*params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
  _fe_problem.getNonlinearSystemBase().setPCSide(getParam<MooseEnum>("pc_side"));

  _fe_problem.getNonlinearSystemBase().setMooseKSPNormType(getParam<MooseEnum>("ksp_norm"));
}

void
MoosePreconditioner::copyVarValues(MeshBase & mesh,
                                   const unsigned int from_system,
                                   const unsigned int from_var,
                                   const NumericVector<Number> & from_vector,
                                   const unsigned int to_system,
                                   const unsigned int to_var,
                                   NumericVector<Number> & to_vector)
{
  {
    MeshBase::node_iterator it = mesh.local_nodes_begin();
    MeshBase::node_iterator it_end = mesh.local_nodes_end();

    for (; it != it_end; ++it)
    {
      Node * node = *it;

      unsigned int n_comp = node->n_comp(from_system, from_var);

      mooseAssert(node->n_comp(from_system, from_var) == node->n_comp(to_system, to_var),
                  "Number of components does not match in each system");

      for (unsigned int i = 0; i < n_comp; i++)
      {
        dof_id_type from_dof = node->dof_number(from_system, from_var, i);
        dof_id_type to_dof = node->dof_number(to_system, to_var, i);

        to_vector.set(to_dof, from_vector(from_dof));
      }
    }
  }
  {
    MeshBase::element_iterator it = mesh.local_elements_begin();
    MeshBase::element_iterator it_end = mesh.local_elements_end();

    for (; it != it_end; ++it)
    {
      Elem * elem = *it;

      unsigned int n_comp = elem->n_comp(from_system, from_var);

      mooseAssert(elem->n_comp(from_system, from_var) == elem->n_comp(to_system, to_var),
                  "Number of components does not match in each system");

      for (unsigned int i = 0; i < n_comp; i++)
      {
        dof_id_type from_dof = elem->dof_number(from_system, from_var, i);
        dof_id_type to_dof = elem->dof_number(to_system, to_var, i);

        to_vector.set(to_dof, from_vector(from_dof));
      }
    }
  }
}
