[CmdLetBinding()]
Param(
    [string]$versionDetailsPath,
    [string]$dependencyName,
    [string]$dependencyVersion
)

[xml]$VersionDetails = Get-Content -Encoding utf8 -Path $VersionDetailsPath

$dependency = $buildConfig.Dependencies.ProductDependencies.Dependency | where {$_.Name -eq "$dependencyName"}
$dependency.Version = $dependencyVersion

$VersionDetails.Save($VersionDetailsPath)
Write-Host "Saved versions back to $VersionDetailsPath"
