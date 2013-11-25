#ifndef __SERVE_H__
#define __SERVE_H__

#include "serve.h"

void
set_status( t_httpreq *req, t_httpresp *resp) {

    if( strcmp(req->version, "0.9" ) == 0 ){
        if (req->valid &&
            req->method_ident == HTTP_GET){
                //200
                resp->status = HTTP_OK;
        }else if (req->valid &&
                     ((req->method_ident == HTTP_POST) ||
                         (req->method_ident == HTTP_HEAD))){
                         //405
                         resp->status = HTTP_METHOD_NOT_ALLOWED;
        }else{
              //400
              resp->status = HTTP_BAD_REQUEST;
        }
    }else if(strcmp(req->version, "1.0") == 0 ){
        if(req->valid  &&
              (req->method_ident == HTTP_GET ||
                  req->method_ident == HTTP_HEAD)){
                  //200
                  resp->status = HTTP_OK;
        }else if(req->valid && req->method_ident == HTTP_POST){
                  //501
                  resp->status = HTTP_NOT_IMPLEMENTED;
        }else if(req->valid == 0){
                  //400
                 resp->status = HTTP_BAD_REQUEST;
        }
   }else if( strcmp(req->version, "1.1") == 0){
        if (req->valid &&
               ((req->method_ident == HTTP_GET) ||
                   (req->method_ident == HTTP_HEAD) ||
                       req->method_ident == HTTP_POST)){
                       //505
                       resp->status = HTTP_VERSION_NOT_SUPPORTED;
        }else{
            //400
            resp->status = HTTP_BAD_REQUEST;
        }

    }else{
        //500
        resp->status = HTTP_SERVER_ERROR;
    }

}

#endif
