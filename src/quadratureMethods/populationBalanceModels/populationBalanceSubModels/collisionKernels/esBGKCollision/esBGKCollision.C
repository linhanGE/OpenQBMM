/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2018 Alberto Passalacqua
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

#include "esBGKCollision.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace populationBalanceSubModels
{
namespace collisionKernels
{
    defineTypeNameAndDebug(esBGKCollision, 0);

    addToRunTimeSelectionTable
    (
        collisionKernel,
        esBGKCollision,
        dictionary
    );
}
}
}


// * * * * * * * * * * * * * * Static Functions  * * * * * * * * * * * * * * //

void Foam::populationBalanceSubModels::collisionKernels::esBGKCollision
::updateCells1D(const label celli)
{
    const volVectorMomentFieldSet& moments = quadrature_.moments();

    scalar m0 = max(moments(0)[celli], SMALL);

    // Mean velocity
    scalar u = meanVelocity(m0, moments(1)[celli]);
    scalar uSqr = sqr(u);

    // Variances of velocities
    scalar sigma = max(moments(2)[celli]/m0 - uSqr, 0.0);
    scalar sigma1 = (a1_ + b1_)*sigma;
    Theta_[celli] = sigma1;

    Meq_(0) = moments(0)[celli];
    Meq_(1) = moments(1)[celli];
    Meq_(2) = m0*(sigma1 + uSqr);
    Meq_(3) = m0*(3.0*sigma1*u + u*uSqr);
    Meq_(4) = m0*(6.0*uSqr*sigma1 + 3.0*sqr(sigma1) + uSqr*uSqr);
}

void Foam::populationBalanceSubModels::collisionKernels::esBGKCollision
::updateCells2D(const label celli)
{
    const volVectorMomentFieldSet& moments = quadrature_.moments();
    scalar m00 = max(moments(0,0)[celli], SMALL);

    // Mean velocity
    scalar u = meanVelocity(m00, moments(1,0)[celli]);
    scalar v = meanVelocity(m00, moments(0,1)[celli]);
    scalar uSqr = sqr(u);
    scalar vSqr = sqr(v);

    // Variances of velocities
    scalar sigma1 = max(moments(2,0)[celli]/m00 - uSqr, 0.0);
    scalar sigma2 = max(moments(0,2)[celli]/m00 - vSqr, 0.0);
    Theta_[celli] = (sigma1 + sigma2)/2.0;

    scalar sigma11 = a1_*Theta_[celli] + b1_*sigma1;
    scalar sigma22 = a1_*Theta_[celli] + b1_*sigma2;
    scalar sigma12 = b1_*(moments(1,1)[celli]/m00 - u*v);

    Meq_(0,0) = moments(0,0)[celli];
    Meq_(1,0) = moments(1,0)[celli];
    Meq_(0,1) = moments(0,1)[celli];
    Meq_(2,0) = m00*(sigma11 + uSqr);
    Meq_(1,1) = m00*(sigma12 + u*v);
    Meq_(0,2) = m00*(sigma22 + vSqr);
    Meq_(3,0) = m00*(3.0*sigma11*u + u*uSqr);
    Meq_(0,3) = m00*(3.0*sigma22*v + v*vSqr);
    Meq_(4,0) = m00*(6.0*uSqr*sigma11 + 3.0*sqr(sigma11) + uSqr*uSqr);
    Meq_(0,4) = m00*(6.0*vSqr*sigma22 + 3.0*sqr(sigma22) + vSqr*vSqr);
}

void Foam::populationBalanceSubModels::collisionKernels::esBGKCollision
::updateCells3D(const label celli)
{
    const volVectorMomentFieldSet& moments = quadrature_.moments();
    scalar m000 = max(moments(0,0,0)[celli], SMALL);

    // Mean velocity
    scalar u = meanVelocity(m000, moments(1,0,0)[celli]);
    scalar v = meanVelocity(m000, moments(0,1,0)[celli]);
    scalar w = meanVelocity(m000, moments(0,0,1)[celli]);
    scalar uSqr = sqr(u);
    scalar vSqr = sqr(v);
    scalar wSqr = sqr(w);

    // Variances of velocities
    scalar sigma1 = max(moments(2,0,0)[celli]/m000 - uSqr, 0.0);
    scalar sigma2 = max(moments(0,2,0)[celli]/m000 - vSqr, 0.0);
    scalar sigma3 = max(moments(0,0,2)[celli]/m000 - wSqr, 0.0);
    Theta_[celli] = (sigma1 + sigma2 + sigma3)/3.0;

    scalar sigma11 = a1_*Theta_[celli] + b1_*sigma1;
    scalar sigma22 = a1_*Theta_[celli] + b1_*sigma2;
    scalar sigma33 = a1_*Theta_[celli] + b1_*sigma3;
    scalar sigma12 = b1_*(moments(1,1,0)[celli]/m000 - u*v);
    scalar sigma13 = b1_*(moments(1,0,1)[celli]/m000 - u*w);
    scalar sigma23 = b1_*(moments(0,1,1)[celli]/m000 - v*w);

    Meq_(0,0,0) = moments(0,0,0)[celli];
    Meq_(1,0,0) = moments(1,0,0)[celli];
    Meq_(0,1,0) = moments(0,1,0)[celli];
    Meq_(0,0,1) = moments(0,0,1)[celli];
    Meq_(2,0,0) = m000*(sigma11 + uSqr);
    Meq_(1,1,0) = m000*(sigma12 + u*v);
    Meq_(1,0,1) = m000*(sigma13 + u*w);
    Meq_(0,2,0) = m000*(sigma22 + vSqr);
    Meq_(0,1,1) = m000*(sigma23 + v*w);
    Meq_(0,0,2) = m000*(sigma33 + wSqr);
    Meq_(3,0,0) = m000*(3.0*sigma11*u + u*uSqr);
    Meq_(0,3,0) = m000*(3.0*sigma22*v + v*vSqr);
    Meq_(0,0,3) = m000*(3.0*sigma33*w + w*wSqr);
    Meq_(4,0,0) = m000*(6.0*uSqr*sigma11 + 3.0*sqr(sigma11) + uSqr*uSqr);
    Meq_(0,4,0) = m000*(6.0*vSqr*sigma22 + 3.0*sqr(sigma22) + vSqr*vSqr);
    Meq_(0,0,4) = m000*(6.0*wSqr*sigma33 + 3.0*sqr(sigma33) + wSqr*wSqr);
}


void Foam::populationBalanceSubModels::collisionKernels::esBGKCollision
::updateFields1D()
{
    const volVectorMomentFieldSet& moments = quadrature_.moments();
    volScalarField m0(max(moments(0), SMALL));

    // Mean velocity
    volScalarField u(meanVelocity(m0, moments(1)));
    volScalarField uSqr(sqr(u));

    // Variances of velocities
    dimensionedScalar zeroVar("zero", sqr(dimVelocity), 0.0);
    volScalarField sigma(max(moments(2)/m0 - uSqr, zeroVar));
    volScalarField sigma1((a1_ + b1_)*sigma);
    Theta_ = sigma1;

    Meqf_(0) = moments(0);
    Meqf_(1) = moments(1);
    Meqf_(2) = m0*(sigma1 + uSqr);
    Meqf_(3) = m0*(3.0*sigma1*u + u*uSqr);
    Meqf_(4) = m0*(6.0*uSqr*sigma1 + 3.0*sqr(sigma1) + uSqr*uSqr);
}

void Foam::populationBalanceSubModels::collisionKernels::esBGKCollision
::updateFields2D()
{
    const volVectorMomentFieldSet& moments = quadrature_.moments();
    volScalarField m00(max(moments(0,0), SMALL));

    // Mean velocity
    volScalarField u(meanVelocity(m00, moments(1,0)));
    volScalarField v(meanVelocity(m00, moments(0,1)));
    volScalarField uSqr(sqr(u));
    volScalarField vSqr(sqr(v));

    // Variances of velocities
    dimensionedScalar zeroVar("zero", sqr(dimVelocity), 0.0);
    volScalarField sigma1(max(moments(2,0)/m00 - uSqr, zeroVar));
    volScalarField sigma2(max(moments(0,2)/m00 - vSqr, zeroVar));
    Theta_ = (sigma1 + sigma2)/2.0;

    volScalarField sigma11(a1_*Theta_ + b1_*sigma1);
    volScalarField sigma22(a1_*Theta_ + b1_*sigma2);
    volScalarField sigma12(b1_*(moments(1,1)/m00 - u*v));

    Meqf_(0,0) = moments(0,0);
    Meqf_(1,0) = moments(1,0);
    Meqf_(0,1) = moments(0,1);
    Meqf_(2,0) = m00*(sigma11 + uSqr);
    Meqf_(1,1) = m00*(sigma12 + u*v);
    Meqf_(0,2) = m00*(sigma22 + vSqr);
    Meqf_(3,0) = m00*(3.0*sigma11*u + u*uSqr);
    Meqf_(0,3) = m00*(3.0*sigma22*v + v*vSqr);
    Meqf_(4,0) = m00*(6.0*uSqr*sigma11 + 3.0*sqr(sigma11) + uSqr*uSqr);
    Meqf_(0,4) = m00*(6.0*vSqr*sigma22 + 3.0*sqr(sigma22) + vSqr*vSqr);
}

void Foam::populationBalanceSubModels::collisionKernels::esBGKCollision
::updateFields3D()
{
    const volVectorMomentFieldSet& moments = quadrature_.moments();
    volScalarField m000(max(moments(0,0,0), SMALL));

    // Mean velocity
    volScalarField u(meanVelocity(m000, moments(1,0,0)));
    volScalarField v(meanVelocity(m000, moments(0,1,0)));
    volScalarField w(meanVelocity(m000, moments(0,0,1)));
    volScalarField uSqr(sqr(u));
    volScalarField vSqr(sqr(v));
    volScalarField wSqr(sqr(w));

    // Variances of velocities
    dimensionedScalar zeroVar("zero", sqr(dimVelocity), 0.0);
    volScalarField sigma1(max(moments(2,0,0)/m000 - uSqr, zeroVar));
    volScalarField sigma2(max(moments(0,2,0)/m000 - vSqr, zeroVar));
    volScalarField sigma3(max(moments(0,0,2)/m000 - wSqr, zeroVar));
    Theta_ = (sigma1 + sigma2 + sigma3)/3.0;

    volScalarField sigma11(a1_*Theta_ + b1_*sigma1);
    volScalarField sigma22(a1_*Theta_ + b1_*sigma2);
    volScalarField sigma33(a1_*Theta_ + b1_*sigma3);
    volScalarField sigma12(b1_*(moments(1,1,0)/m000 - u*v));
    volScalarField sigma13(b1_*(moments(1,0,1)/m000 - u*w));
    volScalarField sigma23(b1_*(moments(0,1,1)/m000 - v*w));

    Meqf_(0,0,0) = moments(0,0,0);
    Meqf_(1,0,0) = moments(1,0,0);
    Meqf_(0,1,0) = moments(0,1,0);
    Meqf_(0,0,1) = moments(0,0,1);
    Meqf_(2,0,0) = m000*(sigma11 + uSqr);
    Meqf_(1,1,0) = m000*(sigma12 + u*v);
    Meqf_(1,0,1) = m000*(sigma13 + u*w);
    Meqf_(0,2,0) = m000*(sigma22 + vSqr);
    Meqf_(0,1,1) = m000*(sigma23 + v*w);
    Meqf_(0,0,2) = m000*(sigma33 + wSqr);
    Meqf_(3,0,0) = m000*(3.0*sigma11*u + u*uSqr);
    Meqf_(0,3,0) = m000*(3.0*sigma22*v + v*vSqr);
    Meqf_(0,0,3) = m000*(3.0*sigma33*w + w*wSqr);
    Meqf_(4,0,0) = m000*(6.0*uSqr*sigma11 + 3.0*sqr(sigma11) + uSqr*uSqr);
    Meqf_(0,4,0) = m000*(6.0*vSqr*sigma22 + 3.0*sqr(sigma22) + vSqr*vSqr);
    Meqf_(0,0,4) = m000*(6.0*wSqr*sigma33 + 3.0*sqr(sigma33) + wSqr*wSqr);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::populationBalanceSubModels::collisionKernels::esBGKCollision
::esBGKCollision
(
    const dictionary& dict,
    const fvMesh& mesh,
    const velocityQuadratureApproximation& quadrature,
    const bool ode
)
:
    collisionKernel(dict, mesh, quadrature, ode),
    e_(dict.lookupType<scalar>("e")),
    b_(dict.lookupOrDefault<scalar>("b", 0)),
    Theta_
    (
        IOobject
        (
            "esBGK:Theta",
            mesh.time().timeName(),
            mesh
        ),
        mesh,
        dimensionedScalar("0", sqr(dimVelocity), 0.0)
    ),
    dp_("d", dimLength, dict),
    zeta_(dict_.lookupOrDefault("zeta", 1.0)),
    Meqf_(quadrature.moments().size(), momentOrders_),
    Meq_(quadrature.moments().size(), momentOrders_)
{
    scalar omega = (1.0 + e_)/2.0;
    scalar gamma = 1.0 - b_;
    a1_ = gamma*sqr(omega);
    b1_ = a1_ - 2.0*gamma*omega + 1.0;

    if (!ode)
    {
        forAll(Meqf_, mi)
        {
            const labelList& momentOrder = momentOrders_[mi];
            Meqf_.set
            (
                momentOrder,
                new volScalarField
                (
                    IOobject
                    (
                        "Meq" + mappedList<scalar>::listToWord(momentOrder),
                        mesh_.time().timeName(),
                        mesh_
                    ),
                    mesh_,
                    dimensionedScalar
                    (
                        "zero",
                        quadrature.moments()[mi].dimensions(),
                        0.0
                    )
                )
            );
        }
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::populationBalanceSubModels::collisionKernels::esBGKCollision
::~esBGKCollision()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::populationBalanceSubModels::collisionKernels::esBGKCollision
::updateCells
(
    const label celli
)
{
    if (nDimensions_ == 1)
    {
        updateCells1D(celli);
    }
    else if (nDimensions_ == 2)
    {
        updateCells2D(celli);
    }
    else if (nDimensions_ == 3)
    {
        updateCells3D(celli);
    }
}

void Foam::populationBalanceSubModels::collisionKernels::esBGKCollision
::updateFields()
{
    if (nDimensions_ == 1)
    {
        updateFields1D();
    }
    else if (nDimensions_ == 2)
    {
        updateFields2D();
    }
    else if (nDimensions_ == 3)
    {
        updateFields3D();
    }
}

Foam::scalar
Foam::populationBalanceSubModels::collisionKernels::esBGKCollision
::explicitCollisionSource(const label mi, const label celli) const
{
    scalar c = quadrature_.moments()[0][celli]/0.63;
    scalar gs0 = (2.0 - c)/(2.0*pow3(1.0 - c)) + 1.1603*c;
    scalar tauC =
        zeta_*sqrt(Foam::constant::mathematical::pi)*dp_.value()
       /max
        (
            12.0*gs0*quadrature_.moments()[0][celli]*sqrt(Theta_[celli]),
            1e-10
        );

    return (Meq_[mi] - quadrature_.moments()[mi][celli])/tauC;
}

Foam::tmp<Foam::fvScalarMatrix>
Foam::populationBalanceSubModels::collisionKernels::esBGKCollision
::implicitCollisionSource(const volVectorMoment& m) const
{
    volScalarField c(quadrature_.moments()[0]/0.63);
    volScalarField gs0((2.0 - c)/(2.0*pow3(1.0 - c)) + 1.1603*c);
    volScalarField tauC
    (
        zeta_*sqrt(Foam::constant::mathematical::pi)*dp_
       /max
        (
            12.0*gs0*quadrature_.moments()[0]*sqrt(Theta_),
            dimensionedScalar("small", dimVelocity, 1e-10)
        )
    );

    return
    (
        Meqf_(m.cmptOrders())/tauC
      - fvm::Sp(1/tauC, m)
    );
}
// ************************************************************************* //
