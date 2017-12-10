
void output_vcxproj(MemoryArena &arena, String dependecy_folder, FolderArray &folder_array, FileArray &file_array) {
	{
		char *filepath = "game_plugin.vcxproj";
		MAKE_OUTPUT_FILE(output, filepath);

fprintf(output, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
fprintf(output, "<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n");
fprintf(output, "  <ItemGroup Label=\"ProjectConfigurations\">\n");
fprintf(output, "    <ProjectConfiguration Include=\"Debug|Win32\">\n");
fprintf(output, "      <Configuration>Debug</Configuration>\n");
fprintf(output, "      <Platform>Win32</Platform>\n");
fprintf(output, "    </ProjectConfiguration>\n");
fprintf(output, "    <ProjectConfiguration Include=\"Dev|x64\">\n");
fprintf(output, "      <Configuration>Dev</Configuration>\n");
fprintf(output, "      <Platform>x64</Platform>\n");
fprintf(output, "    </ProjectConfiguration>\n");
fprintf(output, "    <ProjectConfiguration Include=\"Release|Win32\">\n");
fprintf(output, "      <Configuration>Release</Configuration>\n");
fprintf(output, "      <Platform>Win32</Platform>\n");
fprintf(output, "    </ProjectConfiguration>\n");
fprintf(output, "  </ItemGroup>\n");
fprintf(output, "  <PropertyGroup Label=\"Globals\">\n");
fprintf(output, "    <ProjectGuid>{2F0CEB4F-DEBA-4017-AA9E-F041FCFEAC64}</ProjectGuid>\n");
fprintf(output, "    <ProjectName>game_plugin</ProjectName>\n");
fprintf(output, "  </PropertyGroup>\n");
fprintf(output, "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n");
fprintf(output, "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\" Label=\"Configuration\">\n");
fprintf(output, "    <ConfigurationType>DynamicLibrary</ConfigurationType>\n");
fprintf(output, "    <UseOfMfc>false</UseOfMfc>\n");
fprintf(output, "    <CharacterSet>MultiByte</CharacterSet>\n");
fprintf(output, "    <PlatformToolset>v110</PlatformToolset>\n");
fprintf(output, "  </PropertyGroup>\n");
fprintf(output, "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Dev|x64'\" Label=\"Configuration\">\n");
fprintf(output, "    <ConfigurationType>DynamicLibrary</ConfigurationType>\n");
fprintf(output, "    <UseOfMfc>false</UseOfMfc>\n");
fprintf(output, "    <CharacterSet>MultiByte</CharacterSet>\n");
fprintf(output, "    <PlatformToolset>v110</PlatformToolset>\n");
fprintf(output, "  </PropertyGroup>\n");
fprintf(output, "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\" Label=\"Configuration\">\n");
fprintf(output, "    <ConfigurationType>DynamicLibrary</ConfigurationType>\n");
fprintf(output, "    <UseOfMfc>false</UseOfMfc>\n");
fprintf(output, "    <CharacterSet>MultiByte</CharacterSet>\n");
fprintf(output, "    <PlatformToolset>v110</PlatformToolset>\n");
fprintf(output, "  </PropertyGroup>\n");
fprintf(output, "  <PropertyGroup Label=\"Configuration\" Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">\n");
fprintf(output, "    <PlatformToolset>v110</PlatformToolset>\n");
fprintf(output, "  </PropertyGroup>\n");
fprintf(output, "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n");
fprintf(output, "  <ImportGroup Label=\"ExtensionSettings\">\n");
fprintf(output, "  </ImportGroup>\n");
fprintf(output, "  <ImportGroup Label=\"PropertySheets\">\n");
fprintf(output, "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n");
fprintf(output, "  </ImportGroup>\n");
fprintf(output, "  <PropertyGroup Label=\"UserMacros\" />\n");
fprintf(output, "  <PropertyGroup>\n");
fprintf(output, "    <IncludePath>$(VCInstallDir)include;$(VCInstallDir)atlmfc\\include;$(WindowsSDK_IncludePath);%s</IncludePath>\n", *dependecy_folder);
fprintf(output, "  </PropertyGroup>\n");
fprintf(output, "  <ItemDefinitionGroup>\n");
fprintf(output, "    <ClCompile>\n");
fprintf(output, "      <PreprocessorDefinitions>ARROWHEAD</PreprocessorDefinitions>\n");
fprintf(output, "    </ClCompile>\n");
fprintf(output, "  </ItemDefinitionGroup>\n");
fprintf(output, "  <ItemGroup>\n");
		for (int i = 0; i < file_array.count; ++i) {
			File &file = file_array.entries[i];
fprintf(output, "    <ClInclude Include=\"%s\" />\n", *file.path);
		}
fprintf(output, "  </ItemGroup>\n");
fprintf(output, "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n");
fprintf(output, "  <ImportGroup Label=\"ExtensionTargets\">\n");
fprintf(output, "  </ImportGroup>\n");
fprintf(output, "</Project>\n");
		fclose(output);
	}


	{
		char *filepath = "game_plugin.vcxproj.filters";
		MAKE_OUTPUT_FILE(output, filepath);

fprintf(output, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
fprintf(output, "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n");
fprintf(output, "  <ItemGroup>\n");

		for (int i = 0; i < folder_array.count; ++i) {
			String &folder = folder_array.entries[i];
			GUID guid;
			CoCreateGuid(&guid);
			char guid_buffer[40];
			sprintf(guid_buffer, "%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x",
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
				guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

fprintf(output, "    <Filter Include=\"%s\">\n", *folder);
fprintf(output, "      <UniqueIdentifier>{%s}</UniqueIdentifier>\n", guid_buffer);
fprintf(output, "    </Filter>\n");
		}
fprintf(output, "  </ItemGroup>\n");
fprintf(output, "  <ItemGroup>\n");
		for (int i = 0; i < file_array.count; ++i) {
			File &file = file_array.entries[i];
			if (file.folder.length > 0) {
fprintf(output, "    <ClInclude Include=\"%s\">\n", *file.path);
fprintf(output, "      <Filter>%s</Filter>\n", *file.folder);
fprintf(output, "    </ClInclude>\n");
			}
		}
fprintf(output, "  </ItemGroup>\n");
fprintf(output, "</Project>\n");

		fclose(output);
	}
}
