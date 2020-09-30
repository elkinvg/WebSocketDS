#ifndef ERROR_TYPE_H
#define ERROR_TYPE_H

namespace WebSocketDS_ns
{
    enum class ERROR_TYPE {
        INIT_FAILED,            // ������ ������������� ��������������� �������
        AUTH_CHECK,             // ������ ��������������. �������� ����� ��� ������ TODO: \|/ ����������?
        AUTH_PERM,              // ������ ��������������. ��� �������
        AUTH_SERVER_ERR,        // ������ ��������������. ������ ����������
        IS_NOT_VALID,           // ������������ ������
        NOT_SUPP,               // ��� ���������
        NOT_SUPP_IN_CURR,       // ��� ��������� � ������� ������
        UNKNOWN_REQ_TYPE,       // ����������� ���� �������
        CHECK_REQUEST,          // ��������� ������. ������������ ������, ��� �� ������ ����������� ����
        TANGO_EXCEPTION,        // TANGO exception
        UNAVAILABLE_DEVS,       // All device unavailable
        EVENT_ERR,              // ���������� �� ���������� ������
        CHECK_CODE,             // ������ ��� ������ �� ������ ���������� ��� ���������� ������
        SUBSCR_NOT_FOUND,       // �� ������ ��������� � ������ id
        COMMUNICATION_FAILED,   // Tango::CommunicationFailed
        CONNECTION_FAILED,      // Tango::ConnectionFailed
        UNKNOWN_EXC,            // ���������� ������������ ����
        NOT_SUBSCR_YET,         // ��� ��� �����������
        FROM_EVENT_SUBSCR,      // ������ ��������� ��� �������� �� ������� � ��������� ������
        DEVICE_NOT_IN_GROUP     // ����� ������� ��� � ������
    };
}

#endif