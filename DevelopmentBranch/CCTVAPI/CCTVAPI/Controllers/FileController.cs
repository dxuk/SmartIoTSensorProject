using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web;
using System.Web.Http;

namespace CCTVAPI.Controllers
{

/*
<!DOCTYPE html>
<html>
<head>
    <title></title>
    <meta charset="utf-8" />
    <script src="Scripts/jquery-1.10.2.min.js"></script>
</head>
<body>
    <form>
        <div>
            <label for="fileUpload" />
            Select File to Upload: <input id="fileUpload" type="file" />

            <input id="btnUploadFile" type="button" value="Upload File" />
        </div>
    </form>
</body>
</html>
<script type="text/javascript">
    $(document).ready(function () {

        $('#btnUploadFile').on('click', function () {

            var data = new FormData();

            var files = $("#fileUpload").get(0).files;

            // Add the uploaded image content to the form data collection
            if (files.length > 0) {
                data.append("UploadedImage", files[0]);
            }

            // Make Ajax request with the contentType = false, and procesDate = false
            $.ajax({
                type: "POST",
                url: "/api/file/uploadfile",
                contentType: false,
                processData: false,
                data: data
            });
        });
    });
</script>
*/

    public class FileController : ApiController
    {
        [HttpPost]
        public void UploadFile()
        {
            if (HttpContext.Current.Request.Files.AllKeys.Any())
            {
                // Get the uploaded image from the Files collection
                var httpPostedFile = HttpContext.Current.Request.Files[0];

                if (httpPostedFile != null)
                {
                    // Validate the uploaded image(optional)

                    // Get the complete file path
                    var fileSavePath = Path.Combine(HttpContext.Current.Server.MapPath("~/UploadedFiles"), httpPostedFile.FileName);

                    // Save the uploaded file to "UploadedFiles" folder
                    httpPostedFile.SaveAs(fileSavePath);
                }
            }
        }
    }
}
