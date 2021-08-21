#include "AudioAsset.h"
#include "../Logging/Logging.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>


AudioAsset* AudioAsset::TryLoad(std::filesystem::path Path)
{
	if (!std::filesystem::exists(Path))
	{
		Logging::LogError("GameAsset::TryLoad()", "Could not find asset at path " + Path.string());
		return nullptr;
	}

	std::ifstream FileStream(Path.string());

	AudioAsset* LoadedAsset = new AudioAsset();

	LoadedAsset->FileSize = std::filesystem::file_size(Path);

	LoadedAsset->bDataAssigned = true;
	LoadedAsset->AssetData = new uint8_t[LoadedAsset->FileSize];
	LoadedAsset->FilePath = Path;

	LoadedAsset->MixChunk = Mix_LoadWAV((char*)Path.u8string().c_str());

	LoadedGameAssets.push_back(LoadedAsset);
	return LoadedAsset;
	return nullptr;
}

Mix_Chunk* AudioAsset::GetMixChunk()
{
	return MixChunk;
}