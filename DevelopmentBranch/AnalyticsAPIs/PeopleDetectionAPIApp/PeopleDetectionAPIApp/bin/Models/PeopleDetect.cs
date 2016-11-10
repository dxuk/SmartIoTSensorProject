using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Drawing;

namespace PeopleDetectionAPIApp.Models
{
    public class PeopleDetect
    {
        public int counted { get; set; }
        public Rectangle[] rectCoord { get; set; }
       
    }
}