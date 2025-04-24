#pragma once
#include "IAssetBaker.h"
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Logger.h"

#include "Nuake/FileSystem/File.h"

#include <chrono>
#include <filesystem>
#include <map>
#include <string>
#include <thread>

namespace Nuake
{
	class AssetBakerManager
	{
		private:
		std::map<std::string, Ref<IAssetBaker>> Bakers;
		
		public:
		static AssetBakerManager& Get()
		{
			static AssetBakerManager instance;
			return instance;
		}
		
		void RegisterBaker(Ref<IAssetBaker> baker)
		{
			// Latest registered baker overwrites previous
			for(const auto& extension : baker->GetExtensions())
			{
				Bakers[extension] = baker;
			}
		}
		
		void OnAssetReimport(const Ref<File>& file)
		{
			const std::string extension = file->GetExtension();
			if (Bakers.find(extension) == Bakers.end())
			{
				return;
			}

			Bakers[extension]->Bake(file);
			Logger::Log("New file baked: " + file->GetRelativePath(), "baker", VERBOSE);
		}

		bool WaitForFileReady(const std::string& path, int timeoutMs = 10000)
		{
			using namespace std::chrono_literals;
			namespace fs = std::filesystem;

			const int checkIntervalMs = 500;
			auto startTime = std::chrono::steady_clock::now();
			std::uintmax_t lastSize = 0;

			while (true)
			{
				if (!fs::exists(path))
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
					continue;
				}

				auto currentSize = fs::file_size(path);
				if (currentSize == lastSize && currentSize > 0)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
					if (fs::file_size(path) == currentSize)
						return true;
				}

				lastSize = currentSize;

				if (std::chrono::steady_clock::now() - startTime > std::chrono::milliseconds(timeoutMs))
					return false;

				std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
			}
		}

		void OnNewAssetDetected(const Ref<File>& file)
		{
			const std::string extension = file->GetExtension();
			if (Bakers.find(extension) == Bakers.end())
			{
				return;
			}

			// Wait until the file is fully copied/written
			if (!WaitForFileReady(file->GetAbsolutePath()))
			{
				Logger::Log("File was not ready in time: " + file->GetRelativePath(), "baker", WARNING);
				return;
			}

			Bakers[extension]->Bake(file);
			Logger::Log("New file baked: " + file->GetRelativePath(), "baker", VERBOSE);
		}

		Ref<File> Bake(const Ref<File>& file)
		{
			const std::string extension = file->GetExtension();
			if(Bakers.find(extension) == Bakers.end())
			{
				return file;
			}
			
			return Bakers[extension]->Bake(file);
		}
	};
}