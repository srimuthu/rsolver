#pragma once
#include "RsolverTypes.h"
#include <string>
#include <map>

namespace Rsolver {
	namespace Helpers
	{
		Colors GetDefaultColorForCubeFace(CubeFaces cubeFace);
		CubeFaces GetDefaultCubeFaceForColor(Colors color);
		
		std::string GetLetterForCubeFace(CubeFaces cubeFace);

		FaceColorVector GetDefaultFaceColorVectorForColor(Colors color);
		CubeStateInColors GetDefaultSolvedCubeStateInColors();

		CubeState ParseCubeStatesTxtFile(const std::string& filename);
		std::string GetOrientationForKey(const CubeState& cubeState, const std::string& key, CubeType type);
		std::string GetKeyForOrientation(const CubeState& cubeState, const std::string& orientation, CubeType type);

		CubeStateInUFRDBL GetCubeStateInUFRDBL(const CubeStateInColors& cubeStateInColors,
			const CubeStateInUFRDBL& solutionState,
			const CubeState& defaultStateMap);

		std::string GetEdgeOrientationFromCubeStateInColors(const CubeStateInColors& cubeStateInColors, const EdgeCube& edgeCube);
		std::string GetCornerOrientationFromCubeStateInColors(const CubeStateInColors& cubeStateInColors, const CornerCube& cornerCube);

		std::vector<std::string> SplitStringBySpace(const std::string& str);

		std::string ltrim(const std::string& s);
		std::string rtrim(const std::string& s);
		std::string trim(const std::string& s);
	}
}
