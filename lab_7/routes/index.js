var express = require('express');
var fs = require('fs');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {
  res.sendFile('weather.html',{root: 'public'});
});
router.get('/reverse',function(req,res,next){
	var s = req.query.words;
	s = s.split('').reverse().join('');
	res.status(200).json(s);

});
router.get('/getcity',function(req,res,next) {
            console.log("In getcity route");
         fs.readFile(__dirname + '/cities.dat.txt',function(err,data) {
            if(err) throw err;
            var cities = data.toString().split("\n");
	    var myRe = new RegExp("^" + req.query.q);
            console.log(myRe);
          
	var jsonresult = [];
        for(var i = 0; i < cities.length; i++) {
          var result = cities[i].search(myRe); 
          if(result != -1) {
           console.log(cities[i]);
           jsonresult.push({city:cities[i]});
          } 
        }   
        res.status(200).json(jsonresult);	
         })
 
});

module.exports = router;
