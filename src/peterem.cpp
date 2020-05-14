// Please ignore this file, it is only used for small WIP tests

#include "peterem.hpp"

void optimize_test(const std::string& generator) {
   Solved_Body* solved_body = solved_body_generator()->get(generator,false);
   solved_body->print();
   const int n = solved_body->n;

   assert(solved_body->bcount ==1);
   assert(solved_body->type[0] == &Polytope_T);
   Polytope* p = (Polytope*)solved_body->body[0];
   
   Polytope* q = optimize_polytope(p);
   std::cout << "\n## Optimized: \n\n";
   Polytope_T.print(q);
}


int main(int argc, char** argv) {
   CLI cli(argc,argv,"peterem");
   CLIFunctionsVolume cliFun(cli);
   
   cliFun.claimOpt('b',"body generation configuration");

   std::string generator = "cube";
   auto &gen_map = solved_body_generator()->gen_map();
   cliFun.add(new CLIF_Option<std::string>(&generator,'b',"generator","cube_r1.0_10", gen_map));
   
   bool polytopeTranspose = false;
   cliFun.add(new CLIF_Option<bool>(&polytopeTranspose,'b',"polytopeTranspose","false", {
                                                    {"false",{false, "Polytope format / rows"}},
       					     {"true",{true, "PolytopeT format / columns"}} }));

   
   int v1;
   double v2;
   cliFun.add(new CLIF_DoubleOption<int,double>(&v1,&v2,'e',"choice","c1", {
			   {"c1", {{1,1.1}, "desc c1"}},
			   {"c2", {{2,2.1}, "desc c2"}},
			   }));

   int v3;
   double v4;
   size_t v5;
   cliFun.add(new CLIF_TrippleOption<int,double,size_t>(&v3,&v4,&v5,'e',"choice2","c1", {
			   {"c1", {{1,{1.1, (size_t)0x1 << 35}}, "desc c1"}},
			   {"c2", {{1,{2.1, (size_t)0x2 << 35}}, "desc c2"}},
			   }));




   cliFun.preParse();
   if (!cli.parse()) {return -1;}
   cliFun.postParse();
   
   std::cout << "choice "<< v1 << " " << v2 << "\n";
   std::cout << "choice "<< v3 << " " << v4 << " " << v5 << "\n";
   
   optimize_test(generator);
   return 1;

   Solved_Body* solved_body = solved_body_generator()->get(generator,polytopeTranspose);
   solved_body->print();
   const int n = solved_body->n;

   FT* p = (FT*)(aligned_alloc(32, n*sizeof(FT))); // align this to 32
   
   Ellipsoid* e = Ellipsoid_new(n);

   auto f = [&](FT x, FT y, FT z, FT &r, FT &g, FT &b) {
      FT xx = x*4*n-2*n;
      FT yy = y*4*n-2*n;
      FT zz = (z*4*n-2*n)/4.0;
      for(int i=0;i<n;i++) {p[i] = (i==0)*xx + (i==1)*yy + (i==2)*zz;}
     
      FT eval = Ellipsoid_eval(e,p);
      r = 0;
      g = eval*(eval<=1.0);
      b = eval/(4*n*n)*(eval <= 4*n*n);
      

      auto &type = solved_body->type;
      auto &body = solved_body->body;
      auto &bcount = solved_body->bcount;
      for(int c=0;c<bcount;c++) {
         if(type[c]->inside(body[c],p)) {
	    r += 1.0/bcount;
	 }
      }
   };

   {// ---------------------- IMG
      int width = 800;
      int height = 800;
      int depth = 100;
      EVP::Image_BMP img(height,width);
      for(int z=0;z<depth; z++) {
         for(int i=0; i<height; i++){
             for(int j=0; j<width; j++){
                 FT rr,gg,bb;
                 f((double)i/height, (double)j/width, (double)z/depth, rr,gg,bb);
                 int r = (unsigned char)(rr*255); ///red
                 int g = (unsigned char)(gg*255); ///green
                 int b = (unsigned char)(bb*255); ///blue
                 img.set(j,i,r,g,b);
             }
         }
         std::string num = std::to_string(z);
	 while(num.size() < 5) {num = "0"+num;}
         img.toFile("out/peterem_out_"+num+".bmp");
      }
   }// ---------------------- END IMG

   #ifdef NDEBUG
   std::cout<< "## WARNING: DEBUG DISABLED!\n";
   #else
   std::cout<< "## TESTS COMPLETE.\n";
   #endif
}




