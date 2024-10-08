/*
----o0o=================================================================o0o----
* Copyright (c) 2006, Ian Parberry
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the University of North Texas nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
----o0o=================================================================o0o----
*/

#include "VertexBufferBase.h"
#include "common/CommonStuff.h"

extern LPDIRECT3DDEVICE9 pD3DDevice;

VertexBufferBase::VertexBufferBase(int count, bool isDynamic, DWORD fvf, int vertexStride)
: ResourceBase(isDynamic),
  m_count(count),
  m_bufferLocked(false),
  m_isDynamic(isDynamic),
  m_FVF(fvf),
  m_vertexStride(vertexStride)
{
}

VertexBufferBase::~VertexBufferBase()
{
  release();
}

bool VertexBufferBase::lock()
{
  if(m_dxBuffer == NULL || m_bufferLocked)
  {
    return false;
  }

  if( FAILED( m_dxBuffer->Lock(
    0, 0, (void**)(&m_data), 0/*m_isDynamic ? D3DLOCK_DISCARD : 0*/) ) )
  {
    // you may want to abort here
    return false;
  }

  m_bufferLocked = true;
  m_dataEmpty = false;
  return true;
}

bool VertexBufferBase::unlock()
{
  if(m_dxBuffer == NULL || !m_bufferLocked)
  {
    return false;
  }

  if( FAILED( m_dxBuffer->Unlock() ) )
  {
    return false;
  }

  m_bufferLocked = false;
  return true;
}

void VertexBufferBase::release()
{
  if(m_dxBuffer != NULL)
  {
    m_dxBuffer->Release();
    m_dxBuffer = NULL;
  }
}

void VertexBufferBase::restore()
{
  if(pD3DDevice == NULL)
  {
    ABORT("VertexBufferBase::restore() failed since pD3DDevice was NULL");
  }

  if(m_isDynamic)
  {
    if( FAILED( pD3DDevice->CreateVertexBuffer(
      m_vertexStride * m_count,
      D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
      m_FVF,
      D3DPOOL_DEFAULT,
      &m_dxBuffer,
      NULL) ) )
    {
      m_dxBuffer = NULL;
      ABORT("VertexBufferBase::restore() failed to create DirectX vertex buffer");
    }
  }
  else
  {
    if( FAILED( pD3DDevice->CreateVertexBuffer(
      m_vertexStride * m_count,
      0,
      m_FVF,
      D3DPOOL_MANAGED,
      &m_dxBuffer,
      NULL) ) )
    {
      m_dxBuffer = NULL;
      ABORT("VertexBufferBase::restore() failed to create DirectX vertex buffer");
    }
  }

  m_bufferLocked = false;
  m_dataEmpty = true;
}