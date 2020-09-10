// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Rainer Gericke
// =============================================================================
// Header for an helper class defining common methods for shape node classes
// =============================================================================

#ifndef CH_VSG_BOX_H
#define CH_VSG_BOX_H

#include <iostream>
#include "chrono/core/ChVector.h"
#include "chrono/physics/ChSystemNSC.h"
#include "chrono_vsg/core/ChApiVSG.h"
#include "chrono_vsg/shapes/ChVSGIndexedMesh.h"

#include <vsg/all.h>

namespace chrono {
namespace vsg3d {

class CH_VSG_API VSGBox : public ChVSGIndexedMesh {
  public:
    VSGBox();
    virtual void Initialize(vsg::vec3& lightPosition, ChVSGPhongMaterial& mat, std::string& texFilePath) override;
};
}  // namespace vsg3d
}  // namespace chrono
#endif