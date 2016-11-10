using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web.Http;
using Swashbuckle.Swagger.Annotations;
using System.Drawing;
using Emgu.CV;
using Emgu.CV.CvEnum;
using Emgu.CV.Structure;
using Emgu.CV.UI;
using Emgu.CV.Cuda;
using PedestrianDetection;
using System.Net;
using PeopleDetectionAPIApp.Models;
using System.Web;
using System.IO;

namespace PeopleDetectionAPIApp.Controllers
{
    public class peopleDetectController : ApiController
    {
        // POST api/peopleDetect
        [SwaggerOperation("PeopleDetect")]
        [SwaggerResponse(HttpStatusCode.OK)]
        public PeopleDetect Post(string url)
        {
            PeopleDetect results = new PeopleDetect();
            results.counted = 0;
            var webClient = new WebClient();

            //download image from url 
            byte[] imageBytes = webClient.DownloadData(url);
            Mat image = new Mat();
            CvInvoke.Imdecode(imageBytes, LoadImageType.Color, image);

            bool tryUseCuda = false;
            long processingTime;

            //find the pedestrians in the image and return rectangles and count
            results.rectCoord = FindPedestrian.Find(image, tryUseCuda, out processingTime);
            foreach (Rectangle rect in results.rectCoord)
            {
                results.counted = results.counted + 1;
            }

            return results;                 
            
        }       

    }

    public class detectController : ApiController
    {
        [HttpPost]
        public PeopleDetect UploadFile()
        {
            if (HttpContext.Current.Request.Files.AllKeys.Any())
            {
                // Get the uploaded image from the Files collection
                var httpPostedFile = HttpContext.Current.Request.Files[0];

                if (httpPostedFile != null)
                {
                    //get the file path and save the image
                    var fileSavePath = Path.Combine(HttpContext.Current.Server.MapPath("~/UploadedFiles"), httpPostedFile.FileName);
                    httpPostedFile.SaveAs(fileSavePath);

                    PeopleDetect results = new PeopleDetect();
                    results.counted = 0;

                    Mat image = new Mat(fileSavePath, LoadImageType.Color);

                    bool tryUseCuda = false;
                    long processingTime;

                    //find the pedestrians in the image and return rectangles and count
                    results.rectCoord = FindPedestrian.Find(image, tryUseCuda, out processingTime);
                    foreach (Rectangle rect in results.rectCoord)
                    {
                        results.counted = results.counted + 1;
                    }

                    File.Delete(fileSavePath);

                    return results;

                }
                else
                {
                    PeopleDetect results = new PeopleDetect();
                    results.counted = 0;
                    return results;
                }
            }
            else
            {
                PeopleDetect results = new PeopleDetect();
                results.counted = 0;
                return results;
            }
        }
    }
}
