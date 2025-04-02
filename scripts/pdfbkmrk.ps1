<#
.SYNOPSIS
    Add bookmarks to a PDF file.
.DESCRIPTION
    Use this script to add bookmarks to a PDF from a sidecar text file.
.EXAMPLE
    pdfbkmrk C:\Users\Brian\Desktop\Input.pdf C:\Users\Brian\Desktop\Output.pdf
.NOTES
    Author: Brian Hanke
    Date:   April 2, 2025
#>

Param(
    [parameter(Position=0,Mandatory,HelpMessage="Input file")][string]$i,
    [parameter(Position=1,Mandatory,HelpMessage="Output file")][string]$o
)

$sidecar = "C:\Source\bhgc-pdfbkmrk\scripts\bookmarks.txt"

Write-Host("Processing $i...") -ForegroundColor yellow

gswin64c.exe -dBATCH -dNOPAUSE -dQUIET -sDEVICE=pdfwrite -sOutputFile="$o" $i $sidecar

Write-Host("Done! Output file $o saved to disk.") -ForegroundColor green