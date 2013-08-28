// recipe.cpp
// Implements the RecipeLibrary class

#include "recipe.h"

#include <map>
#include <sstream>

#include "error.h"
#include "logger.h"

namespace cyclus {
namespace recipe {

void Load(Context* ctx, QueryEngine* qe) {
  bool atom_basis;
  std::string basis_str = qe->GetElementContent("basis");
  if (basis_str == "atom") {
    atom_basis = true;
  } else if (basis_str == "mass") {
    atom_basis = false;
  } else {
    throw IOError(basis_str + " basis is not 'mass' or 'atom'.");
  }

  std::string name = qe->GetElementContent("name");
  CLOG(LEV_DEBUG3) << "loading recipe: " << name
                   << " with basis: " << basis_str;

  double value;
  int key;
  std::string query = "isotope";
  int nIsos = qe->NElementsMatchingQuery(query);
  CompMap v;
  for (int i = 0; i < nIsos; i++) {
    QueryEngine* isotope = qe->QueryElement(query, i);
    key = strtol(isotope->GetElementContent("id").c_str(), NULL, 10);
    value = strtod(isotope->GetElementContent("comp").c_str(), NULL);
    v[key] = value;
    CLOG(LEV_DEBUG3) << "  Isotope: " << key << " Value: " << v[key];
  }

  if (atom_basis) {
    ctx->RegisterRecipe(name, Composition::CreateFromAtom(v));
  } else {
    ctx->RegisterRecipe(name, Composition::CreateFromMass(v));
  }
}

} // namespace recipe
} // namespace cyclus
