/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2016-2017 Alberto Passalacqua
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

Class
    Foam::momentGenerationSubModels::velocity

Description
    Moment generation done using volume fraction, density, and bubble diameter.

SourceFiles
    velocity.C

\*---------------------------------------------------------------------------*/

#ifndef velocity_H
#define velocity_H

#include "momentGenerationModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace momentGenerationSubModels
{

/*---------------------------------------------------------------------------*\
                    Class velocity Declaration
\*---------------------------------------------------------------------------*/

class velocity
:
    public momentGenerationModel
{
public:

        //- Runtime type information
        TypeName("velocity");


    // Constructors

        //- Construct from components
        velocity
        (
            const fvMesh& mesh,
            const dictionary& dict,
            const labelListList& momentsOrders,
            const label nNodes
        );


    //- Destructor
    virtual ~velocity();


    // Public member functions

        //- Set the node quanties for a region given a dictionary
        virtual void setNodes(const dictionary& dict);

        //- Update weights and abscissae given a dictionary
        virtual void updateMoments(const label celli);

        //- Update moments given a dictionary
        virtual void updateMoments
        (
            const label patchi,
            const label facei
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace momentGenerationSubModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
