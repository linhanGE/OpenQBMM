/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2015-2017 Alberto Passalacqua
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is derivative work of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "LuoSvendsen.H"
#include "addToRunTimeSelectionTable.H"
#include "fundamentalConstants.H"
#include "phaseModel.H"
#include "PhaseCompressibleTurbulenceModel.H"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace bubbleBreakupKernels
{
    defineTypeNameAndDebug(LuoSvendsen, 0);

    addToRunTimeSelectionTable
    (
        bubbleBreakupKernel,
        LuoSvendsen,
        dictionary
    );
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::bubbleBreakupKernels::LuoSvendsen::LuoSvendsen
(
    const dictionary& dict,
    const fvMesh& mesh
)
:
    bubbleBreakupKernel(dict, mesh),
    fluid_(mesh.lookupObject<twoPhaseSystem>("phaseProperties")),
    Cf_
    (
        dict.lookupOrDefault
        (
            "Cf",
            dimensionedScalar("Cf", dimless, 0.26)
        )
    ),
    epsilonf_
    (
        IOobject
        (
            "LuoSvendsen:epsilonf",
            fluid_.mesh().time().timeName(),
            fluid_.mesh()
        ),
        fluid_.mesh(),
        dimensionedScalar("zero", sqr(dimVelocity)/dimTime, 0.0)
    ),
    de_
    (
        IOobject
        (
            "LuoSvendsen:de",
            fluid_.mesh().time().timeName(),
            fluid_.mesh()
        ),
        fluid_.mesh(),
        dimensionedScalar("zero", dimLength, 0.0)
    )
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::bubbleBreakupKernels::LuoSvendsen::~LuoSvendsen()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::bubbleBreakupKernels::LuoSvendsen::update()
{
    const phaseModel& phase(fluid_.phase1());
    epsilonf_ = phase.turbulence().epsilon();
    epsilonf_.max(SMALL);

    de_ = pow(pow3(phase.turbulence().nut())/epsilonf_, 0.25);
}


Foam::tmp<Foam::volScalarField>
Foam::bubbleBreakupKernels::LuoSvendsen::Kb(const label nodei) const
{
    const volScalarField& d = fluid_.phase1().ds(nodei);
    const volScalarField& rho2 = fluid_.phase2().rho();
    const dimensionedScalar& sigma = fluid_.sigma();
    volScalarField xi(de_/d + SMALL);

    tmp<volScalarField> breakupSource =
        0.923*fluid_.phase2()*cbrt(epsilonf_*d)
       *sqr(1.0 + xi)/(sqr(d)*pow(xi, 20.0))
       *exp
        (
            12.0*Cf_*sigma
           /(2.045*rho2*pow(xi, 11.0/3.0)*pow(d, 5.0/3.0)*pow(epsilonf_, 2.0/3.0))
        );
    breakupSource.ref().dimensions().reset(inv(dimTime));
    return breakupSource;
}

Foam::scalar
Foam::bubbleBreakupKernels::LuoSvendsen::Kb
(
    const label celli,
    const label nodei
) const
{
    scalar d(fluid_.phase1().ds(nodei)[celli]);
    scalar rho2(fluid_.phase2().rho()[celli]);
    scalar sigma(fluid_.sigma().value());
    scalar xi = max(de_[celli]/d, 20.0);


    return
        0.923*fluid_.phase2()[celli]*cbrt(epsilonf_[celli]*d)
       *sqr(1.0 + xi)/(sqr(d)*pow(xi, 11.0/3.0))
       *exp
        (
            -12.0*Cf_.value()*sigma
           /(
               2.045*rho2
              *pow(xi, 11.0/3.0)*pow(d, 5.0/3.0)*pow(epsilonf_[celli], 2.0/3.0)
            )
        );
}

// ************************************************************************* //
