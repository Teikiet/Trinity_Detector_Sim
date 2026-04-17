/*
 * $Id: VException.cpp,v 1.2 2012/11/16 17:37:45 sembrosk Exp $
 *
 * VException.cpp -- base-class for all VERITAS exceptions, implementation
 * by Filip Pizlo, 2002, 2003
 */

#include "VException.h"

void VException::makeTotal() const {
    std::ostringstream ret;
    
    ret<<desc.c_str();
    
    if (msg.length()!=0) {
        ret<<": "<<msg.c_str();
    }
    
    if (hasThrowLocation()) {
        ret<<": thrown in "<<throw_file<<" on line "<<throw_line;
    }
    
    for (std::vector< std::string >::const_iterator
         i=comments.begin();
         i!=comments.end();
         ++i)
    {
        ret<<": "<<*i;
    }
    
    std::string last_cmnt_str=last_comment.str();
    if (last_cmnt_str.length()>0) {
        ret<<": "<<last_cmnt_str;
    }
    
    total=ret.str();
}

VException::VException(const std::string &exception_type,
                       const std::string &thrown_by):
    desc(exception_type),
    throw_line(0)
{
    msg.append("In ");
    msg.append(thrown_by);
}

VException::VException():
    throw_line(0)
{
}

VException::VException(const VException &other):
    desc(other.desc),
    msg(other.msg),
    throw_file(other.throw_file),
    throw_line(other.throw_line),
    comments(other.comments)
{
    last_comment.str(other.last_comment.str());
    endComment();
}

VException::~VException()  noexcept {
}

void VException::endComment()  noexcept {
    std::string last_cmnt_str=last_comment.str();
    last_comment.str("");
    if (last_cmnt_str.length()>0) {
        comments.push_back(last_cmnt_str);
    }
}

void VException::addComment(const std::string &comment) {
    endComment();
    comments.push_back(comment);
}

void VException::addComment(const char *comment) {
    endComment();
    if (comment!=NULL) {
        comments.push_back(std::string(comment));
    }
}

void VException::setThrowLocation(const char *filename,
                                  unsigned line_no)
{
    throw_file=filename;
    throw_line=line_no;
}

void VException::addRethrowLocation(const char *filename,
                                    unsigned line_no)
{
    // do nothing just yet.
}

const std::vector< std::string > VException::getComments() const {
    std::vector< std::string > ret(comments);
    std::string last_cmnt_str=last_comment.str();
    if (last_cmnt_str.length()>0) {
        ret.push_back(last_cmnt_str);
    }
    return ret;
}

std::ostream& operator<<(std::ostream& stream,const VException &x) {
    stream<<"Exception: "<<x.getType()<<": "<<x.getMessage()<<std::endl;
    
    if (x.hasThrowLocation()) {
        stream<<"Code location: "<<x.getThrowFile()
              <<":"<<x.getThrowLine()<<std::endl;
    }
    
    x.printTypeSpecificExceptionDetails(stream);
    
    std::vector< std::string > comments=x.getComments();
    for (std::vector< std::string >::iterator
         i=comments.begin();
         i!=comments.end();
         ++i)
    {
        stream<<*i<<std::endl;
    }
    
    return stream;
}



// **************************************************************************
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 
// 2011, 2012 Purdue University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, 
//  this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are 
// those of the authors and should not be interpreted as representing official
// policies, either expressed or implied, of Purdue University or the VERITAS 
// Collaboration.
// **************************************************************************
