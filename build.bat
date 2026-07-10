echo ::group::nuget restore
nuget restore G2O.sln || exit /b
echo ::endgroup::

echo ::group::Build .NET 4.8
msbuild G2O.sln /p:Configuration=net48-release /p:Platform=x64 || exit /b
echo ::endgroup::

echo ::group::Build .NET Core 3.1
msbuild G2O.sln /p:Configuration=netcoreapp31-release /p:Platform=x64 || exit /b
echo ::endgroup::

echo ::group::Build .NET Core 6
msbuild G2O.sln /p:Configuration=net6-release /p:Platform=x64 || exit /b
echo ::endgroup::

echo ::group::Test .NET Core 6
dotnet test Fugro\Test\bin\net6-release\net6.0\Fugro.G2O.Test.dll || exit /b
echo ::endgroup::

echo ::group::Pack NuGet
nuget pack Fugro\G2O\Fugro.G2O.nuspec || exit /b
echo ::endgroup::