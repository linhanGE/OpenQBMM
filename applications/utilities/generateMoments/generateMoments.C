/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2016-2017 Alberto Passalacqua
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

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

Application
    Application to construct moments give other parameters. Builds initial
    field files for main solver to use as input.

Description
    Preprocessing application to eliminate the need to create fields for all
    moments. Instead moments are consucted using inputs from
    momentGenerationDict. Different methods can be used.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "momentGenerationModel.H"
#include "mappedPtrList.H"
#include "topoSetSource.H"
#include "cellSet.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"

    Info<< "Reading initial conditions and creating moments" << nl << endl;

    IOdictionary phaseDicts
    (
        IOobject
        (
            "momentGenerationDict",
            mesh.time().system(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );

    List<word> phases(phaseDicts.lookup("phases"));
    const dictionary& boundaries(phaseDicts.subDict("boundaries"));

    for (label phasei = 0; phasei < phases.size(); phasei++)
    {
        word phaseName = phases[phasei];
        const dictionary& phaseDict(phaseDicts.subDict(phaseName));

        Info<< "Creating moments for phase: " << phaseName << endl;

        // Read number of nodes from quadratureProperties.phaseName
        IOdictionary quadratureDict
        (
            IOobject
            (
                IOobject::groupName
                (
                    "quadratureProperties",
                    phaseName
                ),
                mesh.time().constant(),
                mesh,
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            )
        );

        labelListList momentOrders(quadratureDict.lookup("moments"));
        labelListList nodeIndexes(quadratureDict.lookup("nodes"));
        label nMoments = momentOrders.size();
        label nNodes = nodeIndexes.size();

        autoPtr<momentGenerationModel> momentGenerator =
            momentGenerationModel::New
            (
                mesh,
                phaseDict,
                momentOrders,
                nNodes
            );


        mappedPtrList<volScalarField> moments(nMoments, momentOrders);

        //  Set internal field values and initialize moments.
        {
            Info<< "Setting internal fields" << nl << endl;
            const dictionary& dict
            (
                phaseDict.found("internal")
              ? phaseDict.subDict("internal")
              : phaseDict.subDict("default")
            );

            momentGenerator().setNodes(dict);

            forAll(moments, mi)
            {
                const labelList& momentOrder= momentOrders[mi];
                moments.set
                (
                    momentOrder,
                    new volScalarField
                    (
                        IOobject
                        (
                            IOobject::groupName
                            (
                                "moment",
                                IOobject::groupName
                                (
                                    mappedPtrList<scalar>::listToWord(momentOrder),
                                    phases[phasei]
                                )
                            ),
                            "0",
                            mesh,
                            IOobject::NO_READ,
                            IOobject::AUTO_WRITE
                        ),
                        mesh,
                        momentGenerator().moments()[mi]
                    )
                );

                //  Set boundaries based oboundary section
                //  Initial values specified in the dictionary are overwritten
                moments[mi].boundaryFieldRef().readField
                (
                    moments[mi].internalField(),
                    boundaries
                );
            }

            forAll(moments[0], celli)
            {
                momentGenerator->updateMoments(celli);
                forAll(moments, mi)
                {
                    moments[mi][celli] = momentGenerator->moments()[mi].value();
                }
            }
        }

        forAll(mesh.boundaryMesh(), bi)
        {
            if (moments[0].boundaryField()[bi].fixesValue())
            {
                Info<< "Setting " << mesh.boundaryMesh()[bi].name()
                    << " boundary" << endl;
                const dictionary& dict
                (
                    phaseDict.found(mesh.boundaryMesh()[bi].name())
                  ? phaseDict.subDict(mesh.boundaryMesh()[bi].name())
                  : phaseDict.subDict("default")
                );

                momentGenerator().setNodes(dict);

                forAll(moments, mi)
                {
                    forAll(moments[mi].boundaryField()[bi], facei)
                    {
                        momentGenerator().updateMoments(bi, facei);

                        moments[mi].boundaryFieldRef()[bi][facei] =
                            (momentGenerator().moments()[mi]).value();
                    }
                }
            }
        }

        //- Set regions of domain using methods seen in setFields
        if (phaseDict.found("regions"))
        {
            PtrList<entry> regions(phaseDict.lookup("regions"));

            forAll(regions, regionI)
            {
                const entry& region = regions[regionI];

                autoPtr<topoSetSource> source =
                    topoSetSource::New(region.keyword(), mesh, region.dict());

                if (source().setType() == topoSetSource::CELLSETSOURCE)
                {
                    cellSet selectedCellSet
                    (
                        mesh,
                        "cellSet",
                        mesh.nCells()/10+1  // Reasonable size estimate.
                    );

                    source->applyToSet
                    (
                        topoSetSource::NEW,
                        selectedCellSet
                    );

                    const labelList& cells = selectedCellSet.toc();
                    momentGenerator().setNodes(region.dict());

                    forAll(cells, celli)
                    {
                        momentGenerator().updateMoments(cells[celli]);
                        forAll(moments, mi)
                        {
                            moments[mi][cells[celli]] =
                                momentGenerator().moments()[mi].value();
                        }
                    }

                }
                else if (source().setType() == topoSetSource::FACESETSOURCE)
                {
                    FatalErrorInFunction
                        << "Moments must be volume fields."
                        << abort(FatalError);
                }
            }
        }


        forAll(moments, mi)
        {
            moments[mi].write();
        }
    }

    Info<< nl << "End\n" << endl;

    return 0;
}
