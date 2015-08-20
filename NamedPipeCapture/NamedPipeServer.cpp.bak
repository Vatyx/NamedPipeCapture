#include "NamedPipeServer.h"
#include "SmartHandle.h"
#include <AccCtrl.h>
#include <AclAPI.h>

NamedPipeServer::NamedPipeServer(std::shared_ptr<boost::asio::io_service> io,
                                 std::string pipename, ErrorHandler errfcn,
                                 NPsecurityattributes secattr)
   : s_io_service(std::move(io))
   , m_pipeName(std::move(pipename))
   , m_namedPipeHandle()
   , m_namedPipeConnectionNotification()
   , m_errorFcn(std::move(errfcn))
   , m_SA(std::move(secattr))
   , m_isRunning(false)
   , m_bufferSizeBytes(64 * 1024)
{
   m_connectionTest.clear();
}

std::shared_ptr<NamedPipeServer>
NamedPipeServer::Create(std::shared_ptr<boost::asio::io_service> io,
                        std::string pipename, ErrorHandler errfcn)
{
   return std::make_shared<NamedPipeServer>(std::move(io), std::move(pipename),
                                            std::move(errfcn));
}

NamedPipeServer::~NamedPipeServer() { Stop(); }

void NamedPipeServer::ClearFlagOnDisconnect() { m_connectionTest.clear(); }

void NamedPipeServer::Stop()
{
   std::unique_lock<std::mutex> lg(m_mutex);
   m_isRunning = false;
   auto ec = boost::system::error_code(boost::asio::error::operation_aborted,
                                       boost::asio::error::get_system_category());
   if (m_namedPipeHandle)
      m_namedPipeHandle->close(ec);
   if (m_namedPipeConnectionNotification)
      m_namedPipeConnectionNotification->close(ec);
}

void NamedPipeServer::Start(ConnectionInitFcn fcn, bool FirstTime)
{
   std::unique_lock<std::mutex> lg(m_mutex);
   m_isRunning = true;

   auto tempPipeHandle = BuildNamedPipe(FirstTime);

   // if (!m_namedPipeConnectionNotification)
   //{
   //   EventHandle SignalHandle(::CreateEvent(nullptr, false, false, nullptr));
   //   if (!SignalHandle.IsValid())
   //      throw std::bad_alloc(); // resource problem
   //   m_namedPipeConnectionNotification =
   //       std::make_shared<::boost::asio::windows::object_handle>(
   //           *s_io_service, SignalHandle.ReleaseHandle());
   //}

   auto pOvrLappedIO = std::make_shared<OVERLAPPED>();
   ZeroMemory(pOvrLappedIO.get(), sizeof(OVERLAPPED));
   // pOvrLappedIO->hEvent = m_namedPipeConnectionNotification->native_handle();

   m_namedPipeHandle = std::make_shared<NamedPipe::BoostNamedPipeHandle>(
       *s_io_service, tempPipeHandle.ReleaseHandle());

   InitNamedPipeConnect(*pOvrLappedIO, [=](const boost::system::error_code& ec)
                        {
                           HandleNewConnection(ec, pOvrLappedIO, std::move(fcn));
                        });
}

void NamedPipeServer::HandleNewConnection(const boost::system::error_code& ec,
                                          std::shared_ptr<OVERLAPPED> pOverlapped,
                                          ConnectionInitFcn successCBfunc)
{

   DWORD testerr = 0;
   std::unique_lock<std::mutex> lg(m_mutex);
   if (!m_isRunning)
      return;

   // Error Handling
   if (ec)
   {
      lg.unlock();
      if (m_errorFcn)
      {
         auto tmpFcn = m_errorFcn;
         tmpFcn(ec);
      }
      return;
   }

   auto pNewClient = NamedPipe::Create(std::move(*m_namedPipeHandle));
   lg.unlock();

   successCBfunc(pNewClient);

   m_isRunning = false;
   return;
}

bool NamedPipeServer::IsRunning() const { return m_isRunning; }

void NamedPipeServer::SetErrorFcn(ErrorHandler fcn)
{
   std::lock_guard<std::mutex> lg(m_mutex);
   m_errorFcn = std::move(fcn);
}

bool NamedPipeServer::SetNamedPipeSecurity()
{
   struct SidDeleter
   {
      void operator()(PSID sid)
      {
         if (sid)
            FreeSid(sid);
      }
   };
   typedef std::unique_ptr<void, SidDeleter> SidCleaner;

   PSID pNetworkSID = nullptr, pEveryoneSID = nullptr, pLocalSID = nullptr;
   SidCleaner scNetwork(pNetworkSID), scEveryone(pEveryoneSID), scLocal(pLocalSID);

   PACL pACL = nullptr;

   EXPLICIT_ACCESS ea[3] = {};
   SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
   SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
   SID_IDENTIFIER_AUTHORITY SIDLocalNT = SECURITY_LOCAL_SID_AUTHORITY;
   DWORD dwRes = 0;
   ULONG numACE = 2;

   ZeroMemory(&ea, 3 * sizeof(EXPLICIT_ACCESS));

   // All of this stuff is for PLI 17490OSI8, in order to prevent network access to
   // the named pipe.
   if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0,
                                 0, 0, &pEveryoneSID))
   {
      return false;
   }


   ea[0].grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
   ea[0].grfAccessMode = SET_ACCESS;
   ea[0].grfInheritance = NO_INHERITANCE;
   ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
   ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
   ea[0].Trustee.ptstrName = (LPTSTR)pEveryoneSID;

   if (!AllocateAndInitializeSid(&SIDLocalNT, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0,
                                 0, 0, &pLocalSID))
   {
      return false;
   }

   ea[1].grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
   ea[1].grfAccessMode = SET_ACCESS;
   ea[1].grfInheritance = NO_INHERITANCE;
   ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
   ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
   ea[1].Trustee.ptstrName = (LPTSTR)pLocalSID;


   // if (!IsWindowsVistaOrGreater())
   //{
   //   if (!AllocateAndInitializeSid(
   //      &SIDAuthNT, 1, SECURITY_NETWORK_RID, 0, 0, 0, 0, 0, 0, 0, &pNetworkSID))
   //   {
   //      return PIstatus::PI_ERROR;
   //   }

   //   ea[2].grfAccessPermissions = GENERIC_ALL;
   //   ea[2].grfAccessMode = DENY_ACCESS;
   //   ea[2].grfInheritance = NO_INHERITANCE;
   //   ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
   //   ea[2].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
   //   ea[2].Trustee.ptstrName = (LPTSTR) pNetworkSID;
   //   ++numACE;
   //}

   dwRes = SetEntriesInAcl(numACE, ea, nullptr, &pACL);
   if (dwRes != ERROR_SUCCESS)
   {
      return false;
   }

   auto spACL = std::shared_ptr<ACL>(pACL, [](PACL pACL)
                                     {
                                        if (pACL)
                                        {
                                           LocalFree(pACL);
                                        }
                                     });

   auto psd = std::make_shared<SECURITY_DESCRIPTOR>();
   if (psd)
   {
      if (InitializeSecurityDescriptor(psd.get(), SECURITY_DESCRIPTOR_REVISION))
      {
         // add a nullptr disc. ACL to the security descriptor.
         if (SetSecurityDescriptorDacl(psd.get(), true, pACL, false))
         {
            m_SA = NPsecurityattributes(std::move(psd), TRUE, std::move(spACL));
         }
      }
   }

   return true;
}

NamedPipeHandle NamedPipeServer::BuildNamedPipe(bool isInitial)
{
   INT32 openmode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED;
   //if (isInitial)
   //   openmode |= FILE_FLAG_FIRST_PIPE_INSTANCE;

   INT32 pipemode = PIPE_TYPE_MESSAGE | PIPE_WAIT;

   INT32 timeout = 5000;
   auto pipehnd1 = NamedPipeHandle(::CreateNamedPipe(
       m_pipeName.c_str(), openmode, pipemode, 1,
       m_bufferSizeBytes, m_bufferSizeBytes, 300, NULL));
   if (!pipehnd1.IsValid())
   {
      DWORD dwLastError = ::GetLastError();
      // throw something?
   }
   return pipehnd1;
}

void NamedPipeServer::InitNamedPipeConnect(OVERLAPPED& overlappedIO,
                                           ASIOCallBack fcn)
{
   if (!m_connectionTest.test_and_set())
   {
      if (!::ConnectNamedPipe(
              static_cast<HANDLE>(m_namedPipeHandle->native_handle()),
              &overlappedIO))
      {
         DWORD dwConnectNamedPipeErr = ::GetLastError();
         switch (dwConnectNamedPipeErr)
         {
         case ERROR_PIPE_CONNECTED:
         // Set the event. ConnectNamedPipe isn't going to do this for us now...
         // When we call async_wait, another thread (or this one) will pick up the
         // task.
         // SetEvent(m_namedPipeConnectionNotification->native_handle());
         case ERROR_IO_PENDING:
            break;
         default:
            m_connectionTest.clear();
            auto ec = ::boost::system::error_code(dwConnectNamedPipeErr,
                                                  boost::system::system_category());
            s_io_service->post([ec, fcn]
                               {
                                  fcn(ec);
                               });
         }
      }
      HANDLE hndl = NULL;
      if (::DuplicateHandle(::GetCurrentProcess(),
                            m_namedPipeHandle->native_handle(),
                            ::GetCurrentProcess(), &hndl, SYNCHRONIZE, false,
                            DUPLICATE_SAME_ACCESS) == 0)
      {
         throw std::exception("couldn't duplicate handle", ::GetLastError());
      }
      m_namedPipeConnectionNotification =
          std::make_shared<boost::asio::windows::object_handle>(*s_io_service, hndl);

      m_namedPipeConnectionNotification->async_wait(std::move(fcn));
   }
}


NPsecurityattributes::NPsecurityattributes(
    std::shared_ptr<SECURITY_DESCRIPTOR> pDescriptor, BOOL bInherit,
    std::shared_ptr<ACL> pDACL)
   : m_descriptor(std::move(pDescriptor))
   , m_secattr()
   , m_acl(std::move(pDACL))
{
   // assigned the inherited members
   m_secattr.nLength = sizeof SECURITY_ATTRIBUTES;
   m_secattr.lpSecurityDescriptor = m_descriptor.get();
   m_secattr.bInheritHandle = bInherit;
   // see if a security descriptor was provided or not
   if (!m_descriptor)
   {
      m_descriptor = std::make_shared<SECURITY_DESCRIPTOR>();
      m_secattr.lpSecurityDescriptor = m_descriptor.get();
      // if not provided create a security descriptor with nullptr DACL to grant
      // everyone access
      if (!InitializeSecurityDescriptor(m_secattr.lpSecurityDescriptor,
                                        SECURITY_DESCRIPTOR_REVISION) ||
          !SetSecurityDescriptorDacl(m_secattr.lpSecurityDescriptor, TRUE, nullptr,
                                     FALSE))
      {
         m_descriptor.reset();
         m_secattr.lpSecurityDescriptor = nullptr;
      }
   }
}
NPsecurityattributes::~NPsecurityattributes() {}

NPsecurityattributes::NPsecurityattributes(NPsecurityattributes& other)
   : m_descriptor(other.m_descriptor)
   , m_secattr()
   , m_acl(other.m_acl)
{
   m_secattr.nLength = sizeof(SECURITY_ATTRIBUTES);
   m_secattr.lpSecurityDescriptor = m_descriptor.get();
   m_secattr.bInheritHandle = other.m_secattr.bInheritHandle;
}

NPsecurityattributes& NPsecurityattributes::
operator=(const NPsecurityattributes& other)
{
   m_descriptor = other.m_descriptor;
   m_acl = other.m_acl;
   m_secattr.nLength = sizeof(SECURITY_ATTRIBUTES);
   m_secattr.lpSecurityDescriptor = m_descriptor.get();
   m_secattr.bInheritHandle = other.m_secattr.bInheritHandle;
   return *this;
}

NPsecurityattributes::NPsecurityattributes(NPsecurityattributes&& other)
   : m_descriptor(std::move(other.m_descriptor))
   , m_secattr()
   , m_acl(std::move(other.m_acl))
{
   m_secattr.nLength = other.m_secattr.nLength;
   m_secattr.lpSecurityDescriptor = m_descriptor.get();
   m_secattr.bInheritHandle = other.m_secattr.bInheritHandle;

   other.m_secattr = SECURITY_ATTRIBUTES();
}


NPsecurityattributes& NPsecurityattributes::operator=(NPsecurityattributes&& other)
{
   // TODO: remove the following line when we figure out a better accessor for this
   // class
   // than the casts to LPSECURITY_ATTRIBUTES and overloaded adderss-of

   if (&other != *this)
   {
      memset(&m_secattr, 0, sizeof(m_secattr));
      m_descriptor = std::move(other.m_descriptor);
      m_acl = std::move(other.m_acl);
      m_secattr.nLength = other.m_secattr.nLength;
      m_secattr.lpSecurityDescriptor = m_descriptor.get();
      m_secattr.bInheritHandle = other.m_secattr.bInheritHandle;
      memset(&other.m_secattr, 0, sizeof(other.m_secattr));
   }
   return *this;
}

NPsecurityattributes::operator LPSECURITY_ATTRIBUTES() { return &m_secattr; }
LPSECURITY_ATTRIBUTES NPsecurityattributes::operator&() { return &m_secattr; }