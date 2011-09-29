﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

using Scalien;

namespace ScalienClientUnitTesting
{
    [TestClass]
    class UsersTests
    {
        /* DEPRECATED
        [TestMethod]
        public void GetSetSubmitUsers()
        {
            System.Console.WriteLine("TestingGetSetSubmit");

            Users uTest = new Users();
            //uTest.addClient();
            uTest.ResetTables();

            Assert.IsTrue(uTest.TestGetSetSubmit());
        }*/

        [TestMethod]
        public void SimpleTest()
        {
            Users usr = new Users(Config.nodes);
            usr.EmptyAll();

            usr.InsertUsers(1000);
            // bug ?
            long cnt = usr.CountUsers();
            //usr.SubmitAll();
            usr.TestCycle(500);
            
            Assert.IsTrue(usr.IsConsistent());
        }

        private static void TestWorker(Object param)
        {
            int loop = System.Convert.ToInt32(param);
            int users_per_iteration = 100;

            Users usr = new Users(Config.nodes);
            while (loop-- > 0)
            {
                usr.TestCycle(users_per_iteration);
                usr.SubmitAll();
            }
        }

        [TestMethod]
        public void ShortTest_10_Threads()
        {
            int init_users = 1000;
            int threadnum = 10;

            Users usr = new Users(Config.nodes);
            usr.InsertUsers(init_users);

            Thread[] threads = new Thread[threadnum];
            for (int i = 0; i < threadnum; i++)
            {
                threads[i] = new Thread(new ParameterizedThreadStart(TestWorker));
                threads[i].Start(500);
            }

            for (int i = 0; i < threadnum; i++)
            {
                threads[i].Join();
            }

            Assert.IsTrue(usr.IsConsistent());
        }

        //[TestMethod]
        public void ShortTest_100_Threads()
        {
            Users usr = new Users(Config.nodes);
            usr.InsertUsers(1000);

            Assert.IsTrue(usr.IsConsistent());
        }

        //[TestMethod]
        public void ShortTest_1000_Threads()
        {
            Users usr = new Users(Config.nodes);
            usr.InsertUsers(1000);

            Assert.IsTrue(usr.IsConsistent());
        }
    }
}
