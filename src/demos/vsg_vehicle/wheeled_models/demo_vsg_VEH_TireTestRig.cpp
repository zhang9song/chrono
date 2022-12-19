// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2015 projectchrono.org
// All right reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Radu Serban
// =============================================================================
//
// Demonstration of the single-wheel tire test rig.
//
// =============================================================================

#include <algorithm>

#include "chrono/physics/ChSystemNSC.h"
#include "chrono/physics/ChSystemSMC.h"

#include "chrono_vsg/ChVisualSystemVSG.h"

#include "chrono_models/vehicle/hmmwv/HMMWV_ANCFTire.h"
#include "chrono_models/vehicle/hmmwv/HMMWV_FialaTire.h"
#include "chrono_models/vehicle/hmmwv/HMMWV_ReissnerTire.h"
#include "chrono_models/vehicle/hmmwv/HMMWV_RigidTire.h"
#include "chrono_models/vehicle/hmmwv/HMMWV_TMeasyTire.h"
#include "chrono_models/vehicle/hmmwv/HMMWV_Pac89Tire.h"
#include "chrono_models/vehicle/hmmwv/HMMWV_Pac02Tire.h"
#include "chrono_models/vehicle/hmmwv/HMMWV_Wheel.h"

#include "chrono_vehicle/utils/ChUtilsJSON.h"
#include "chrono_vehicle/terrain/RigidTerrain.h"
#include "chrono_vehicle/wheeled_vehicle/tire/ANCFToroidalTire.h"
#include "chrono_vehicle/wheeled_vehicle/test_rig/ChTireTestRig.h"

#ifdef CHRONO_POSTPROCESS
    #include "chrono_postprocess/ChGnuPlot.h"
#endif

#include "demos/vehicle/SetChronoSolver.h"

using namespace chrono;
using namespace chrono::vehicle;
using namespace chrono::vsg3d;

enum class TireType { RIGID, TMEASY, FIALA, PAC89, PAC02, ANCF4, ANCF8, ANCF_TOROIDAL, REISSNER };
TireType tire_type = TireType::TMEASY;

bool use_JSON = true;

int main() {

    // Create wheel and tire subsystems
    auto wheel = chrono_types::make_shared<hmmwv::HMMWV_Wheel>("Wheel");

    std::shared_ptr<ChTire> tire;
    if (tire_type == TireType::ANCF_TOROIDAL) {
        auto ancf_tire = chrono_types::make_shared<ANCFToroidalTire>("ANCFtoroidal tire");
        ancf_tire->SetRimRadius(0.27);
        ancf_tire->SetHeight(0.18);
        ancf_tire->SetThickness(0.015);
        ancf_tire->SetDivCircumference(40);
        ancf_tire->SetDivWidth(8);
        ancf_tire->SetPressure(320e3);
        ancf_tire->SetAlpha(0.15);
        tire = ancf_tire;
    } else if (use_JSON) {
        std::string tire_file;
        switch (tire_type) {
            case TireType::RIGID:
                tire_file = "hmmwv/tire/HMMWV_RigidTire.json";
                break;
            case TireType::TMEASY:
                tire_file = "hmmwv/tire/HMMWV_TMeasyTire.json";
                break;
            case TireType::FIALA:
                tire_file = "hmmwv/tire/HMMWV_FialaTire.json";
                break;
            case TireType::PAC89:
                tire_file = "hmmwv/tire/HMMWV_Pac89Tire.json";
                break;
            case TireType::PAC02:
                tire_file = "hmmwv/tire/HMMWV_Pac02Tire.json";
                break;
            case TireType::ANCF4:
                tire_file = "hmmwv/tire/HMMWV_ANCF4Tire_Lumped.json";
                break;
            case TireType::ANCF8:
                tire_file = "hmmwv/tire/HMMWV_ANCF8Tire_Lumped.json";
                break;
            case TireType::REISSNER:
                tire_file = "hmmwv/tire/HMMWV_ReissnerTire.json";
                break;
        }
        tire = ReadTireJSON(vehicle::GetDataFile(tire_file));
    } else {
        switch (tire_type) {
            case TireType::RIGID:
                tire = chrono_types::make_shared<hmmwv::HMMWV_RigidTire>("Rigid tire");
                break;
            case TireType::TMEASY:
                tire = chrono_types::make_shared<hmmwv::HMMWV_TMeasyTire>("TMeasy tire");
                break;
            case TireType::FIALA:
                tire = chrono_types::make_shared<hmmwv::HMMWV_FialaTire>("Fiala tire");
                break;
            case TireType::PAC89:
                tire = chrono_types::make_shared<hmmwv::HMMWV_Pac89Tire>("Pac89 tire");
                break;
            case TireType::PAC02:
                tire = chrono_types::make_shared<hmmwv::HMMWV_Pac02Tire>("Pac02 tire");
                break;
            case TireType::ANCF4:
                tire = chrono_types::make_shared<hmmwv::HMMWV_ANCFTire>("ANCF tire",
                                                                        hmmwv::HMMWV_ANCFTire::ElementType::ANCF_4);
                break;
            case TireType::ANCF8:
                tire = chrono_types::make_shared<hmmwv::HMMWV_ANCFTire>("ANCF tire",
                                                                        hmmwv::HMMWV_ANCFTire::ElementType::ANCF_8);
                break;
            case TireType::REISSNER:
                tire = chrono_types::make_shared<hmmwv::HMMWV_ReissnerTire>("Reissner tire");
                break;
        }
    }

    // Create system and set solver
    ChSystem* sys = nullptr;
    ChSolver::Type solver_type;
    ChTimestepper::Type integrator_type;
    double step_size;

    if (tire_type == TireType::ANCF4 || tire_type == TireType::ANCF8 || tire_type == TireType::ANCF_TOROIDAL ||
        tire_type == TireType::REISSNER) {
        sys = new ChSystemSMC;
        step_size = 4e-5;
        solver_type = ChSolver::Type::PARDISO_MKL;
        integrator_type = ChTimestepper::Type::EULER_IMPLICIT_PROJECTED;
        std::static_pointer_cast<ChDeformableTire>(tire)->SetContactFaceThickness(0.02);
    } else {
        sys = new ChSystemNSC;
        step_size = 1e-3;
        solver_type = ChSolver::Type::BARZILAIBORWEIN;
        integrator_type = ChTimestepper::Type::EULER_IMPLICIT_LINEARIZED;
    }

    SetChronoSolver(*sys, solver_type, integrator_type);

    // Create and configure test rig
    ChTireTestRig rig(wheel, tire, sys);

    ////rig.SetGravitationalAcceleration(0);
    rig.SetNormalLoad(8000);

    ////rig.SetCamberAngle(+15 * CH_C_DEG_TO_RAD);

    rig.SetTireStepsize(step_size);
    rig.SetTireCollisionType(ChTire::CollisionType::FOUR_POINTS);
    rig.SetTireVisualizationType(VisualizationType::MESH);

    rig.SetTerrainRigid(0.8, 0, 2e7);
    ////rig.SetTerrainSCM(6259.1e3, 5085.6e3, 1.42, 1.58e3, 34.1, 22.17e-3);

    // Set test scenario
    // -----------------

    // Scenario: driven wheel
    ////rig.SetAngSpeedFunction(chrono_types::make_shared<ChFunction_Const>(10.0));
    ////rig.Initialize();

    // Scenario: pulled wheel
    ////rig.SetLongSpeedFunction(chrono_types::make_shared<ChFunction_Const>(1.0));
    ////rig.Initialize();

    // Scenario: imobilized wheel
    ////rig.SetLongSpeedFunction(chrono_types::make_shared<ChFunction_Const>(0.0));
    ////rig.SetAngSpeedFunction(chrono_types::make_shared<ChFunction_Const>(0.0));
    ////rig.Initialize();

    // Scenario: prescribe all motion functions
    //   longitudinal speed: 0.2 m/s
    //   angular speed: 20 RPM
    //   slip angle: sinusoidal +- 5 deg with 5 s period
    rig.SetLongSpeedFunction(chrono_types::make_shared<ChFunction_Const>(0.2));
    rig.SetAngSpeedFunction(chrono_types::make_shared<ChFunction_Const>(20 * CH_C_RPM_TO_RPS));
    rig.SetSlipAngleFunction(chrono_types::make_shared<ChFunction_Sine>(0, 0.2, 5 * CH_C_DEG_TO_RAD));
    rig.Initialize();

    // Scenario: specified longitudinal slip
    ////rig.Initialize(0.2, 0.1);

    // Create the Irrlicht visualization sys
    auto vis = chrono_types::make_shared<ChVisualSystemVSG>();
    vis->AttachSystem(sys);
    //vis->SetCameraVertical(CameraVerticalDir::Z);
    vis->SetWindowSize(800, 600);
    vis->SetWindowTitle("Tire Test Rig");
    vis->AddCamera(ChVector<>(1.0f, 2.5f, 1.5f));
    vis->Initialize();

    // Perform the simulation
    ChFunction_Recorder long_slip;
    ChFunction_Recorder slip_angle;
    ChFunction_Recorder camber_angle;
    ChFunction_Recorder long_force;
    ChFunction_Recorder side_force;
    ChFunction_Recorder vert_force;

    double time = 0;
    double tmax = 10.0;
    size_t frame_num = 0;
    while (vis->Run()) {
        time = sys->GetChTime();
        if(time > tmax) break;
        if (time > 0.5) {
            long_slip.AddPoint(time, tire->GetLongitudinalSlip());
            slip_angle.AddPoint(time, tire->GetSlipAngle() * CH_C_RAD_TO_DEG);
            camber_angle.AddPoint(time, tire->GetCamberAngle() * CH_C_RAD_TO_DEG);
            auto tforce = rig.ReportTireForce();
            long_force.AddPoint(time, tforce.force.x());
            side_force.AddPoint(time, tforce.force.y());
            vert_force.AddPoint(time, tforce.force.z());
        }

        auto x = rig.GetPos().x();
        auto y = rig.GetPos().y();
        auto z = rig.GetPos().z();
        auto eye = ChVector<>(x + 1.0f, y + 2.5f, z + 1.5f);
        auto center = ChVector<>(x, y + 0.25f, z);
        vis->UpdateCamera(eye,center);

        vis->BeginScene();
        if(frame_num % 2 == 0) vis->Render();
        //tools::drawAllContactPoints(vis.get(), 1.0, ContactsDrawMode::CONTACT_NORMALS);
        rig.Advance(step_size);
        vis->EndScene();

        ////std::cout << sys.GetChTime() << std::endl;
        ////auto long_slip = tire->GetLongitudinalSlip();
        ////auto slip_angle = tire->GetSlipAngle();
        ////auto camber_angle = tire->GetCamberAngle();
        ////std::cout << "   " << long_slip << " " << slip_angle << " " << camber_angle << std::endl;
        ////auto tforce = rig.ReportTireForce();
        ////auto frc = tforce.force;
        ////auto pnt = tforce.point;
        ////auto trq = tforce.moment;
        ////std::cout << "   " << frc.x() << " " << frc.y() << " " << frc.z() << std::endl;
        ////std::cout << "   " << pnt.x() << " " << pnt.y() << " " << pnt.z() << std::endl;
        ////std::cout << "   " << trq.x() << " " << trq.y() << " " << trq.z() << std::endl;
        frame_num++;
    }

#ifdef CHRONO_POSTPROCESS
    postprocess::ChGnuPlot gplot_long_slip("tmp1.gpl");
    gplot_long_slip.SetGrid();
    gplot_long_slip.SetLabelX("time (s)");
    gplot_long_slip.SetLabelY("Long. slip ()");
    gplot_long_slip.Plot(long_slip, "", " with lines lt -1 lc rgb'#00AAEE' ");

    postprocess::ChGnuPlot gplot_slip_angle("tmp2.gpl");
    gplot_slip_angle.SetGrid();
    gplot_slip_angle.SetLabelX("time (s)");
    gplot_slip_angle.SetLabelY("Slip angle (deg)");
    gplot_slip_angle.Plot(slip_angle, "", " with lines lt -1 lc rgb'#00AAEE' ");

    postprocess::ChGnuPlot gplot_camber_angle("tmp3.gpl");
    gplot_camber_angle.SetGrid();
    gplot_camber_angle.SetLabelX("time (s)");
    gplot_camber_angle.SetLabelY("Camber angle (deg)");
    gplot_camber_angle.Plot(camber_angle, "", " with lines lt -1 lc rgb'#00AAEE' ");

    postprocess::ChGnuPlot gplot_long_force("tmp4.gpl");
    gplot_long_force.SetGrid();
    gplot_long_force.SetLabelX("time (s)");
    gplot_long_force.SetLabelY("Longitudinal force (N)");
    gplot_long_force.Plot(long_force, "", " with lines lt -1 lc rgb'#00AAEE' ");

    postprocess::ChGnuPlot gplot_side_force("tmp5.gpl");
    gplot_side_force.SetGrid();
    gplot_side_force.SetLabelX("time (s)");
    gplot_side_force.SetLabelY("Side force (N)");
    gplot_side_force.Plot(side_force, "", " with lines lt -1 lc rgb'#00AAEE' ");

    postprocess::ChGnuPlot gplot_vert_force("tmp6.gpl");
    gplot_vert_force.SetGrid();
    gplot_vert_force.SetLabelX("time (s)");
    gplot_vert_force.SetLabelY("Vertical force (N)");
    gplot_vert_force.Plot(vert_force, "", " with lines lt -1 lc rgb'#00AAEE' ");
#endif

    return 0;
}

