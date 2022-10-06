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
// Authors: Radu Serban
// =============================================================================
//
// Chrono demonstration of using contact callbacks for non-smooth contacts
// (complementarity-based).
//
// The global reference frame has Y up.
//
// =============================================================================

#include <cstdio>
#include <cmath>

#include "chrono/utils/ChUtilsCreators.h"
#include "chrono/physics/ChSystemNSC.h"

#include "chrono_vsg/ChVisualSystemVSG.h"

using namespace chrono;
using namespace chrono::vsg3d;

// -----------------------------------------------------------------------------
// Callback class for contact reporting
// -----------------------------------------------------------------------------
class ContactReporter : public ChContactContainer::ReportContactCallback {
public:
    ContactReporter(std::shared_ptr<ChBody> box1, std::shared_ptr<ChBody> box2) : m_box1(box1), m_box2(box2) {}

private:
    virtual bool OnReportContact(const ChVector<>& pA,
            const ChVector<>& pB,
            const ChMatrix33<>& plane_coord,
            const double& distance,
            const double& eff_radius,
            const ChVector<>& cforce,
            const ChVector<>& ctorque,
            ChContactable* modA,
            ChContactable* modB) override {
        // Check if contact involves box1
        if (modA == m_box1.get()) {
            printf("  A contact on Box 1 at pos: %7.3f  %7.3f  %7.3f", pA.x(), pA.y(), pA.z());
        } else if (modB == m_box1.get()) {
            printf("  B contact on Box 1 at pos: %7.3f  %7.3f  %7.3f", pB.x(), pB.y(), pB.z());
        }

        // Check if contact involves box2
        if (modA == m_box2.get()) {
            printf("  A contact on Box 2 at pos: %7.3f  %7.3f  %7.3f", pA.x(), pA.y(), pA.z());
        } else if (modB == m_box2.get()) {
            printf("  B contact on Box 2 at pos: %7.3f  %7.3f  %7.3f", pB.x(), pB.y(), pB.z());
        }

        const ChVector<>& nrm = plane_coord.Get_A_Xaxis();
        printf("  nrm: %7.3f, %7.3f  %7.3f", nrm.x(), nrm.y(), nrm.z());
        printf("  frc: %7.3f  %7.3f  %7.3f", cforce.x(), cforce.y(), cforce.z());
        printf("  trq: %7.3f, %7.3f  %7.3f", ctorque.x(), ctorque.y(), ctorque.z());
        printf("  penetration: %8.4f   eff. radius: %7.3f\n", distance, eff_radius);

        return true;
    }

    std::shared_ptr<ChBody> m_box1;
    std::shared_ptr<ChBody> m_box2;
};

// -----------------------------------------------------------------------------
// Callback class for modifying composite material
// -----------------------------------------------------------------------------
class ContactMaterial : public ChContactContainer::AddContactCallback {
public:
    virtual void OnAddContact(const collision::ChCollisionInfo& contactinfo,
            ChMaterialComposite* const material) override {
        // Downcast to appropriate composite material type
        auto mat = static_cast<ChMaterialCompositeNSC* const>(material);

        // Set different friction for left/right halfs
        float friction = (contactinfo.vpA.z() > 0) ? 0.3f : 0.8f;
        mat->static_friction = friction;
        mat->sliding_friction = friction;
    }
};

int main(int argc, char* argv[]) {
    GetLog() << "Copyright (c) 2017 projectchrono.org\nChrono version: " << CHRONO_VERSION << "\n\n";

    // ----------------
    // Parameters
    // ----------------

    float friction = 0.6f;
    double collision_envelope = .001;

    // -----------------
    // Create the sys
    // -----------------

    ChSystemNSC sys;
    sys.Set_G_acc(ChVector<>(0, -10, 0));

    // Set solver settings
    sys.SetSolverMaxIterations(100);
    sys.SetMaxPenetrationRecoverySpeed(1e8);
    sys.SetSolverForceTolerance(0);

    // --------------------------------------------------
    // Create a contact material, shared among all bodies
    // --------------------------------------------------

    auto material = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    material->SetFriction(friction);

    // ----------
    // Add bodies
    // ----------

    auto container = chrono_types::make_shared<ChBody>();
    sys.Add(container);
    container->SetPos(ChVector<>(0, 0, 0));
    container->SetBodyFixed(true);
    container->SetIdentifier(-1);

    container->SetCollide(true);
    container->GetCollisionModel()->SetEnvelope(collision_envelope);
    container->GetCollisionModel()->ClearModel();
    utils::AddBoxGeometry(container.get(), material, ChVector<>(4, 0.5, 4), ChVector<>(0, -0.5, 0));
    container->GetCollisionModel()->BuildModel();
    container->GetVisualShape(0)->SetColor(ChColor(0.4f, 0.4f, 0.4f));

    auto box1 = chrono_types::make_shared<ChBody>();
    box1->SetMass(10);
    box1->SetInertiaXX(ChVector<>(1, 1, 1));
    box1->SetPos(ChVector<>(-1, 0.21, -1));
    box1->SetPos_dt(ChVector<>(5, 0, 0));

    box1->SetCollide(true);
    box1->GetCollisionModel()->SetEnvelope(collision_envelope);
    box1->GetCollisionModel()->ClearModel();
    utils::AddBoxGeometry(box1.get(), material, ChVector<>(0.4, 0.2, 0.1));
    box1->GetCollisionModel()->BuildModel();
    box1->GetVisualShape(0)->SetColor(ChColor(0.1f, 0.1f, 0.4f));

    sys.AddBody(box1);

    auto box2 = std::shared_ptr<ChBody>(sys.NewBody());
    box2->SetMass(10);
    box2->SetInertiaXX(ChVector<>(1, 1, 1));
    box2->SetPos(ChVector<>(-1, 0.21, +1));
    box2->SetPos_dt(ChVector<>(5, 0, 0));

    box2->SetCollide(true);
    box2->GetCollisionModel()->SetEnvelope(collision_envelope);
    box2->GetCollisionModel()->ClearModel();
    utils::AddBoxGeometry(box2.get(), material, ChVector<>(0.4, 0.2, 0.1));
    box2->GetCollisionModel()->BuildModel();
    box2->GetVisualShape(0)->SetColor(ChColor(0.4f, 0.1f, 0.1f));

    sys.AddBody(box2);

    // -------------------------------
    // Create the visualization window
    // -------------------------------

    auto vis = chrono_types::make_shared<ChVisualSystemVSG>();
    vis->AttachSystem(&sys);
    vis->SetWindowTitle("NSC callbacks");
    vis->AddCamera(ChVector<>(8, 8, -12));
    vis->SetWindowSize(ChVector2<int>(800, 600));
    vis->SetWindowPosition(ChVector2<int>(100, 100));
    vis->SetClearColor(ChColor(0.8,0.85,0.9));
    vis->SetUseSkyBox(true); // use built-in path
    vis->SetCameraVertical(chrono::vsg3d::CameraVerticalDir::Y);
    vis->SetCameraAngleDeg(40.0);
    vis->SetLightIntensity(1.0);
    vis->SetLightDirection(1.5*CH_C_PI_2, CH_C_PI_4);
    vis->SetWireFrameMode(false);
    ChColor red(1.0,0.0,0.0);
    vis->SetDecoGrid(0.5, 0.5, 12, 12, ChCoordsys<>(ChVector<>(0, 0, 0), Q_from_AngX(CH_C_PI_2)), red);
    /*
    irrlicht::tools::drawGrid(vis.get(), 0.5, 0.5, 12, 12,
            ChCoordsys<>(ChVector<>(0, 0, 0), Q_from_AngX(CH_C_PI_2)));
    */
    vis->Initialize();

    // ---------------
    // Simulate sys
    // ---------------

    auto creporter = chrono_types::make_shared<ContactReporter>(box1, box2);

    auto cmaterial = chrono_types::make_shared<ContactMaterial>();
    sys.GetContactContainer()->RegisterAddContactCallback(cmaterial);

    while (vis->Run()) {
        vis->Render();
        sys.DoStepDynamics(1e-3);
        vis->UpdateFromMBS();
        // Process contacts
        std::cout << sys.GetChTime() << "  " << sys.GetNcontacts() << std::endl;
        sys.GetContactContainer()->ReportAllContacts(creporter);

        // Cumulative contact force and torque on boxes (as applied to COM)
        ChVector<> frc1 = box1->GetContactForce();
        ChVector<> trq1 = box1->GetContactTorque();
        printf("  Box 1 contact force at COM: %7.3f  %7.3f  %7.3f", frc1.x(), frc1.y(), frc1.z());
        printf("  contact torque at COM: %7.3f  %7.3f  %7.3f\n", trq1.x(), trq1.y(), trq1.z());
        ChVector<> frc2 = box2->GetContactForce();
        ChVector<> trq2 = box2->GetContactTorque();
        printf("  Box 2 contact force at COM: %7.3f  %7.3f  %7.3f", frc2.x(), frc2.y(), frc2.z());
        printf("  contact torque at COM: %7.3f  %7.3f  %7.3f\n", trq2.x(), trq2.y(), trq2.z());
    }

    return 0;
}
