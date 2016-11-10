using System;
using System.Text;
using System.Net.Http;
using System.Net.Http.Headers;

public static void Run(Stream myBlob, string name, out string outputEventHubMessage, TraceWriter log)
{
    string uri = "http://peopledetectionapiapp.azurewebsites.net/api/detect";

    using (var httpClient = new HttpClient())
    {
        httpClient.BaseAddress = new Uri(uri);
        var fileStream = new StreamContent(myBlob);
        
        using (var content = new MultipartFormDataContent())
        {
            //get response
            fileStream.Headers.ContentType = MediaTypeHeaderValue.Parse("image/jpeg");
            
            content.Add(fileStream, "image", name);
            
            var response = httpClient.PostAsync(uri, content).Result;            
            var responseString = response.Content.ReadAsStringAsync().Result;
            log.Info(responseString);
            
            //write to output thumb
            outputEventHubMessage = responseString;
            
        }
    }
}