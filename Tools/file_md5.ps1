$algorithm = "MD5"
  $Algo=[System.Security.Cryptography.HashAlgorithm]::Create($algorithm) 
  if ($Algo) {
    $files = $args
    if ($files.length -eq 0) {
      $files = (dir)
    }
    foreach ($file in $files) {
      $Fc = gc $file
      if ($Fc.length -gt 0) {  
        $Encoding = New-Object System.Text.ASCIIEncoding 
        $Bytes = $Encoding.GetBytes($Fc)
        $Hash = $Algo.ComputeHash($Bytes)
        $Hashstring = ""
        foreach ($byte in $hash) {$hashstring += $byte.tostring("x2")}
        $out = new-object psobject
        $segs = $file.ToString().Split('\')
        if ($segs.length -gt 0) {
          $out | add-member noteproperty File $segs[-1]
        } else {
          $out | add-member noteproperty File $file
        }
        $out | add-member noteproperty ($algorithm + ' Hash') $Hashstring
        $results += ,$out
      }
    }
  }
  $results