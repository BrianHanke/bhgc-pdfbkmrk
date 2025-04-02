### BHGC PDF Bookmark

Adapted from https://github.com/pra33g/PDFBookmark and heavily revised.

- Install the latest Windows x64 AGPL Ghostscript here: https://www.ghostscript.com/releases/gsdnld.html
- Add \<ghostscriptInstallFolder\>/bin to your Windows PATH.
- Create a bookmarks.txt file in the same folder as pdfbkmrk.exe. Bookmarks are listed one per line and formatted as \<Page Number\> \<Bookmark Name\>. Tab nests a bookmark below the one above.
- Copy the PDF file you want to create bookmarks for to the same folder as pdfbkmrk.exe.
- Run the utility using `pdfbkmrk <inputFile.pdf> <outputFile.pdf>`.
