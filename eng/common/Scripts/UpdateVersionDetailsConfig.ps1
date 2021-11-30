[CmdLetBinding()]
Param(
    [string]$dependencyName,
    [string]$dependencyVersion
)

# Get the root of the repo.
$scriptFullPath =  (split-path -parent $MyInvocation.MyCommand.Definition)
$rootPath = (split-path -parent $scriptFullPath)

Function CheckFile($filename)
{
    if(-not (Test-Path $filename))
    {
        write-host "File not found: $filename"
        exit 1
    }
}

$configFilename = "$rootPath\eng\Version.Details.xml"
CheckFile $configFilename

# Load the build.config, update the requested version entry, then write it back out
$xmldoc = [System.Xml.XmlDocument](Get-Content $configFilename)
$node = $xmldoc.Dependencies.ProductDependencies.Dependency | ?{$_.Name -eq $dependencyName}
$node.Version = $dependencyVersion
$xmldoc.Save($configFilename)

write-host "Updated $configFilename"
