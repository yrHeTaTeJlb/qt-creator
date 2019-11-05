/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "mcusupportrunconfiguration.h"
#include "mcusupportconstants.h"

#include <projectexplorer/projectconfigurationaspects.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/devicesupport/devicemanager.h>
#include <projectexplorer/devicesupport/deviceusedportsgatherer.h>
#include <projectexplorer/project.h>
#include <projectexplorer/runcontrol.h>
#include <projectexplorer/target.h>
#include <cmakeprojectmanager/cmakekitinformation.h>
#include <cmakeprojectmanager/cmaketool.h>

using namespace ProjectExplorer;
using namespace Utils;

namespace McuSupport {
namespace Internal {

static CommandLine flashAndRunCommand(Target *target)
{
    const QString projectName = target->project()->displayName();

    const CMakeProjectManager::CMakeTool *tool =
            CMakeProjectManager::CMakeKitAspect::cmakeTool(target->kit());

    return CommandLine(tool->filePath(), {
                           "--build",
                           ".",
                           "--target",
                           QString("flash_%1_and_bootloader").arg(projectName)
                       });
}

class FlashAndRunConfiguration : public ProjectExplorer::RunConfiguration
{
public:
    FlashAndRunConfiguration(Target *target, Core::Id id)
        : RunConfiguration(target, id)
    {
        auto effectiveFlashAndRunCall = addAspect<BaseStringAspect>();
        effectiveFlashAndRunCall->setLabelText(tr("Effective flash and run call:"));
        effectiveFlashAndRunCall->setDisplayStyle(BaseStringAspect::TextEditDisplay);
        effectiveFlashAndRunCall->setReadOnly(true);

        auto updateConfiguration = [target, effectiveFlashAndRunCall] {
            effectiveFlashAndRunCall->setValue(flashAndRunCommand(target).toUserOutput());
        };

        updateConfiguration();

        connect(target->activeBuildConfiguration(), &BuildConfiguration::buildDirectoryChanged,
                this, updateConfiguration);
        connect(target->project(), &Project::displayNameChanged,
                this, updateConfiguration);
    }
};

class FlashAndRunWorker : public SimpleTargetRunner
{
    void baseDoStart(const Runnable &runnable, const IDevice::ConstPtr &device)
    {
        SimpleTargetRunner::doStart(runnable, device);
    }
public:
    FlashAndRunWorker(RunControl *runControl)
        : SimpleTargetRunner(runControl)
    {
        setStarter([this, runControl] {
            ProjectExplorer::Target *target = runControl->target();
            const CommandLine cmd = flashAndRunCommand(target);
            Runnable r;
            r.workingDirectory =
                    target->activeBuildConfiguration()->buildDirectory().toUserOutput();
            r.setCommandLine(cmd);
            r.environment = target->activeBuildConfiguration()->environment();
            baseDoStart(r, {});
        });
    }
};

RunWorkerFactory::WorkerCreator makeFlashAndRunWorker()
{
    return RunWorkerFactory::make<FlashAndRunWorker>();
}

EmrunRunConfigurationFactory::EmrunRunConfigurationFactory()
    : FixedRunConfigurationFactory(FlashAndRunConfiguration::tr("Flash and run"))
{
    registerRunConfiguration<FlashAndRunConfiguration>(Constants::RUNCONFIGURATION);
    addSupportedTargetDeviceType(Constants::DEVICE_TYPE);
}

} // namespace Internal
} // namespace McuSupport
